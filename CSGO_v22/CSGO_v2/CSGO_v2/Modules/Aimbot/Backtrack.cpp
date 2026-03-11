#include "Backtrack.h"
#include "aimbot.h"
#include "SDK/Globals/Globals.h"
#include "SDK/Entity/localplayer.h"
#include "SDK/Classes/EngineTrace/TraceTypes.h"
#include "lib/Hooks/hook.h"
#include "lib/Configs/config.h"
#include "lib/utils/utils.h"
#include "imgui.h"
#include <cmath>

// Bone indices (same as aimbot.cpp)
constexpr int BT_BONE_HEAD = 8;

void backtrack::Update()
{
	if (!cfg.aimbot.backtrack.Enabled)
		return;

	if (!globals::g_interfaces.Engine->IsInGame() || !hooks::GlobalVars)
		return;

	gEntity* lp = LocalPlayer.Get();
	if (!lp)
		return;

	__try {
		int maxCl = hooks::GlobalVars->maxClients;
		if (maxCl >= MAX_PLAYERS) maxCl = MAX_PLAYERS - 1;

		for (int i = 1; i <= maxCl; i++)
		{
			auto ent = globals::g_interfaces.ClientEntity->GetClientEntity(i);

			if (!ent || !ent->isValidState() || ent->isDormant() || ent == lp)
			{
				records[i].clear();
				continue;
			}

			if (ent->isTeammate() && !cfg.aimbot.FriendlyFire)
			{
				records[i].clear();
				continue;
			}

			float simTime = *(float*)((uintptr_t)ent + offsets::m_flSimulationTime);

			if (!records[i].empty() && records[i].front().simTime == simTime)
				continue;

			BacktrackRecord record;
			record.simTime = simTime;
			record.origin = ent->getAbsOrigin();
			record.valid = true;

			auto& cache = ent->boneCache();
			if (cache.size > BT_BONE_HEAD && BT_BONE_HEAD >= 0)
			{
				record.headPos = cache[BT_BONE_HEAD].GetVecOrgin();

				int bonesToCopy = min(cache.size, 128);
				memcpy(record.boneMatrix, cache.memory, bonesToCopy * sizeof(math::Matrix3x4));
			}
			else
			{
				record.valid = false;
				continue;
			}

			records[i].push_front(record);

			while ((int)records[i].size() > BACKTRACK_MAX_TICKS)
				records[i].pop_back();
		}

		// Prune expired records
		for (int i = 1; i <= maxCl; i++)
		{
			while (!records[i].empty() && !IsTickValid(records[i].back().simTime))
				records[i].pop_back();
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		// Entity freed mid-update, safe to skip
	}
}

bool backtrack::IsTickValid(float simTime)
{
	if (!hooks::GlobalVars)
		return false;

	auto nci = globals::g_interfaces.Engine->GetNetChannelInfo();
	if (!nci)
		return false;

	float latency = nci->GetLatency(FLOW_OUTGOING) + nci->GetLatency(FLOW_INCOMING);

	float maxWindow = cfg.aimbot.backtrack.TimeLimit / 1000.f;
	float correctTime = latency + hooks::GlobalVars->interval_per_tick;
	float deltaTime = correctTime - (hooks::GlobalVars->curtime - simTime);

	return std::abs(deltaTime) <= maxWindow;
}

static math::Vector BT_CalcAimAngles(const math::Vector& source, const math::Vector& destination)
{
	math::Vector delta = destination - source;
	float hypotenuse = std::sqrt(delta.x * delta.x + delta.y * delta.y);

	math::Vector angles;
	angles.x = std::atan2(-delta.z, hypotenuse) * RAD_TO_DEG;
	angles.y = std::atan2(delta.y, delta.x) * RAD_TO_DEG;
	angles.z = 0.0f;
	return angles;
}

bool backtrack::GetBestRecord(gEntity* localPlayer, const math::Vector& localEyePos,
	const math::Vector& viewAngles, float maxFov, BacktrackRecord& outRecord, int& outPlayerIdx)
{
	float bestFov = maxFov;
	bool found = false;

	int maxCl = hooks::GlobalVars->maxClients;
	if (maxCl >= MAX_PLAYERS) maxCl = MAX_PLAYERS - 1;

	for (int i = 1; i <= maxCl; i++)
	{
		int count = (int)records[i].size();
		for (int j = 0; j < count; j++)
		{
			if (j >= (int)records[i].size()) break; // race guard

			const auto& record = records[i][j];
			if (!record.valid || !IsTickValid(record.simTime))
				continue;

			math::Vector aimAngles = BT_CalcAimAngles(localEyePos, record.headPos);
			float fov = (viewAngles - aimAngles).length2D();

			if (fov < bestFov)
			{
				bestFov = fov;
				outRecord = record;
				outPlayerIdx = i;
				found = true;
			}
		}
	}

	return found;
}

void backtrack::Run(CUserCmd* cmd)
{
	if (!cfg.aimbot.backtrack.Enabled)
		return;

	if (!(cmd->buttons & CUserCmd::IN_ATTACK))
		return;

	if (!globals::g_interfaces.Engine->IsInGame() || !hooks::GlobalVars)
		return;

	gEntity* lp = LocalPlayer.Get();
	if (!lp)
		return;

	bool alive = *(int*)((uintptr_t)lp + offsets::deadFlag) == 0;
	if (!alive)
		return;

	__try {
		auto viewOffset = *(math::Vector*)((uintptr_t)lp + offsets::m_vecViewOffset);
		const auto& origin = lp->getAbsOrigin();
		math::Vector eyePos = { origin.x + viewOffset.x, origin.y + viewOffset.y, origin.z + viewOffset.z };
		math::Vector viewAngles = cmd->viewangles;

		float maxFov = cfg.aimbot.Enabled ? cfg.aimbot.FOV : 180.f;

		BacktrackRecord bestRecord;
		int bestPlayerIdx = 0;

		if (GetBestRecord(lp, eyePos, viewAngles, maxFov, bestRecord, bestPlayerIdx))
		{
			Apply(cmd, bestRecord.simTime);
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		// Entity state changed mid-run, skip this tick
	}
}

void backtrack::Apply(CUserCmd* cmd, float simTime)
{
	if (!hooks::GlobalVars)
		return;

	cmd->tick_count = (int)(simTime / hooks::GlobalVars->interval_per_tick) + 1;
}

void backtrack::Clear()
{
	for (auto& r : records)
		r.clear();
}

int backtrack::GetTotalRecords()
{
	int total = 0;
	for (int i = 1; i < MAX_PLAYERS; i++)
		total += (int)records[i].size();
	return total;
}

void backtrack::RenderTicks()
{
	__try {
		auto* dl = ImGui::GetBackgroundDrawList();
		auto& tc = cfg.aimbot.backtrack.TickColor;

		int maxCl = hooks::GlobalVars->maxClients;
		if (maxCl >= MAX_PLAYERS) maxCl = MAX_PLAYERS - 1;

		for (int i = 1; i <= maxCl; i++)
		{
			int count = (int)records[i].size();
			if (count <= 0) continue;

			for (int j = 0; j < count; j++)
			{
				if (j >= (int)records[i].size()) break; // race guard
				const auto& rec = records[i][j];
				if (!rec.valid) continue;

				math::Vector screenPos;
				if (utils::WorldToScreen(rec.headPos, screenPos))
				{
					float alpha = 1.f - (float)j / (float)count;
					ImU32 col = ImColor(tc.r, tc.g, tc.b, tc.a * alpha);
					dl->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), 3.f, col);
				}
			}
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		// Race with CreateMove modifying records — skip this frame
	}
}
