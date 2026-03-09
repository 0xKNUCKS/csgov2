#include "aimbot.h"
#include "SDK/Globals/Globals.h"
#include "SDK/Entity/localplayer.h"
#include "SDK/Classes/EngineTrace/TraceTypes.h"
#include "lib/Hooks/hook.h"
#include <cmath>

// Bone indices for CS:GO skeleton
constexpr int BONE_HEAD = 8;
constexpr int BONE_NECK = 7;
constexpr int BONE_CHEST = 6;
constexpr int BONE_STOMACH = 4;

static int GetBoneIndex(int aimBoneSetting)
{
	switch (aimBoneSetting) {
		case 0: return BONE_HEAD;
		case 1: return BONE_NECK;
		case 2: return BONE_CHEST;
		case 3: return BONE_STOMACH;
		default: return BONE_HEAD;
	}
}

// Movement corrector for when using Silent Aim, using basic maths
void AdjustMovement(CUserCmd* cmd, const math::Vector& oldAngles, const math::Vector& newAngles)
{
	float forward = cmd->forwardmove;
	float side = cmd->sidemove;

	float deltaAngle = newAngles.y - oldAngles.y;
	float radDelta = deltaAngle * DEG_TO_RAD;

	cmd->forwardmove = std::cos(radDelta) * forward + std::cos(radDelta + PI/2) * side;
	cmd->sidemove = std::sin(radDelta) * forward + std::sin(radDelta + PI/2) * side;
}

// Check if we can see the target position from our eye position
bool IsVisible(const math::Vector& eyePos, const math::Vector& targetPos, gEntity* localPlayer, gEntity* target)
{
	if (!globals::g_interfaces.EngineTrace)
		return true;

	Ray_t ray(eyePos, targetPos);
	ITraceFilter filter(localPlayer);
	trace_t trace;

	__try {
		globals::g_interfaces.EngineTrace->TraceRay(ray, MASK_SHOT, filter, trace);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false; // entity freed mid-trace, treat as not visible
	}

	return trace.fraction > 0.97f || trace.pEntity == target;
}

// Compute aim angles from source to destination
math::Vector CalcAimAngles(const math::Vector& source, const math::Vector& destination)
{
	math::Vector delta = destination - source;
	float hypotenuse = std::sqrt(delta.x * delta.x + delta.y * delta.y);

	math::Vector angles;
	angles.x = std::atan2(-delta.z, hypotenuse) * RAD_TO_DEG;
	angles.y = std::atan2(delta.y, delta.x) * RAD_TO_DEG;
	angles.z = 0.0f;
	return angles;
}

// Get bone position with bounds checking
bool GetBonePos(gEntity* ent, int boneIndex, math::Vector& out)
{
	auto& cache = ent->boneCache();
	if (cache.size <= boneIndex || boneIndex < 0)
		return false;
	out = cache[boneIndex].GetVecOrgin();
	return true;
}

// Track shots fired for RCS start bullet
static int shotsFired = 0;
static bool wasShooting = false;

void aimbot::Run(CUserCmd* cmd)
{
	__try {
		aimbot::RunInternal(cmd);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		// Silently catch crashes during map transitions / entity invalidation
	}
}

void aimbot::RunInternal(CUserCmd* cmd)
{
	if (!cfg.aimbot.Enabled)
		return;

	if (!GetAsyncKeyState(cfg.aimbot.Key.virtualKey))
		return;

	if (!globals::g_interfaces.Engine->IsInGame() || !LocalPlayer.Get() || !hooks::GlobalVars)
		return;

	// Track shot count for RCS start bullet
	bool isShooting = (cmd->buttons & cmd->IN_ATTACK) != 0;
	if (isShooting && !wasShooting)
		shotsFired++;
	else if (isShooting)
		shotsFired++;
	if (!isShooting)
		shotsFired = 0;
	wasShooting = isShooting;

	gEntity* bestTarget = nullptr;
	float bestFov = cfg.aimbot.FOV;
	float bestDistance = FLT_MAX;
	math::Vector bestAimAngles = {};
	Target.ent = nullptr;

	// Use netvar-based eye position (safe during map transitions, no virtual call)
	auto viewOffset = *(math::Vector*)((uintptr_t)LocalPlayer.Get() + offsets::m_vecViewOffset);
	const auto& origin = LocalPlayer->getAbsOrigin();
	math::Vector localPos = { origin.x + viewOffset.x, origin.y + viewOffset.y, origin.z + viewOffset.z };
	math::Vector viewAngles = globals::g_interfaces.Engine->GetViewAngles();

	int boneIndex = GetBoneIndex(cfg.aimbot.AimBone);

	// Compensate view angles for punch when doing FOV comparison
	math::Vector compensatedAngles = viewAngles;
	if (cfg.aimbot.RCS) {
		math::Vector punch = LocalPlayer->getAimPunch();
		compensatedAngles.x -= punch.x * cfg.aimbot.RCSAmountX;
		compensatedAngles.y -= punch.y * cfg.aimbot.RCSAmountY;
	}

	for (int i = 1; i <= hooks::GlobalVars->maxClients; i++)
	{
		auto ent = globals::g_interfaces.ClientEntity->GetClientEntity(i);

		if (!ent || !ent->isValidState() || ent->isDormant() ||
			(ent->isTeammate() && !cfg.aimbot.FriendlyFire))
			continue;

		math::Vector targetPos;
		if (!GetBonePos(ent, boneIndex, targetPos))
			continue;

		// Visibility check - skip targets behind walls
		if (cfg.aimbot.VisibilityCheck && !IsVisible(localPos, targetPos, LocalPlayer.Get(), ent))
			continue;

		math::Vector aimAngles = CalcAimAngles(localPos, targetPos);
		float fov = (compensatedAngles - aimAngles).length2D();
		float distance = (targetPos - localPos).length();

		if (fov < bestFov || (fov == bestFov && distance < bestDistance))
		{
			bestFov = fov;
			bestDistance = distance;
			bestTarget = ent;
			bestAimAngles = aimAngles;

			Target.fov = bestFov;
			Target.distance = bestDistance;
			Target.ent = bestTarget;
		}
	}

	if (bestTarget)
	{
		// Apply recoil compensation (only after N shots)
		if (cfg.aimbot.RCS && shotsFired >= cfg.aimbot.RCSStartBullet) {
			math::Vector punch = LocalPlayer->getAimPunch();
			bestAimAngles.x -= punch.x * cfg.aimbot.RCSAmountX;
			bestAimAngles.y -= punch.y * cfg.aimbot.RCSAmountY;
		}

		math::Vector currentAngles = globals::g_interfaces.Engine->GetViewAngles();
		math::Vector delta = bestAimAngles - currentAngles;

		delta.normalizeDeg();
		delta.normalize();

		// Per-axis smooth: base smooth * axis multiplier
		float smoothX = cfg.aimbot.Smooth * cfg.aimbot.SmoothX;
		float smoothY = cfg.aimbot.Smooth * cfg.aimbot.SmoothY;

		math::Vector finalAngles;
		finalAngles.x = currentAngles.x + delta.x / smoothX;
		finalAngles.y = currentAngles.y + delta.y / smoothY;
		finalAngles.z = 0.f;

		bool isSilent = cfg.aimbot.Silent;

		// Auto shoot - fire when we're close enough to the target
		if (cfg.aimbot.AutoShoot) {
			float aimDiff = (finalAngles - bestAimAngles).length2D();
			if (aimDiff < cfg.aimbot.AutoShootFov)
				cmd->buttons |= cmd->IN_ATTACK;
		}

		if (cmd->buttons & cmd->IN_ATTACK)
			cmd->viewangles = isSilent ? bestAimAngles : finalAngles;
		if (!isSilent)
			globals::g_interfaces.Engine->SetViewAngles(finalAngles);

		AdjustMovement(cmd, currentAngles, finalAngles);
	}

	// Standalone RCS (no target needed, just compensate recoil while shooting)
	if (!bestTarget && cfg.aimbot.RCS && cfg.aimbot.StandaloneRCS) {
		static math::Vector lastPunch = {};

		if ((cmd->buttons & cmd->IN_ATTACK) && shotsFired >= cfg.aimbot.RCSStartBullet) {
			math::Vector punch = LocalPlayer->getAimPunch();

			math::Vector punchDelta;
			punchDelta.x = (punch.x - lastPunch.x) * cfg.aimbot.RCSAmountX;
			punchDelta.y = (punch.y - lastPunch.y) * cfg.aimbot.RCSAmountY;
			punchDelta.z = 0.f;

			// Apply RCS smoothing
			if (cfg.aimbot.RCSSmooth > 1.0f) {
				punchDelta.x /= cfg.aimbot.RCSSmooth;
				punchDelta.y /= cfg.aimbot.RCSSmooth;
			}

			math::Vector angles = globals::g_interfaces.Engine->GetViewAngles();
			angles.x -= punchDelta.x;
			angles.y -= punchDelta.y;
			angles.normalize();

			globals::g_interfaces.Engine->SetViewAngles(angles);
			cmd->viewangles = angles;

			lastPunch = punch;
		} else if (!(cmd->buttons & cmd->IN_ATTACK)) {
			lastPunch = {};
		}
	}
}
