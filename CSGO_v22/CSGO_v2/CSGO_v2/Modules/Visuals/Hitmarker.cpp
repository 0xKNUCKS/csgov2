#include "Hitmarker.h"
#include "SDK/Classes/GameEventManager.h"
#include "SDK/Globals/Globals.h"
#include "lib/Configs/config.h"
#include "lib/Error/CrashLog.h"
#include "dx9/Drawing/drawing.h"
#include "imgui.h"
#include <cmath>
#include <string>

// Hit record for rendering
struct HitRecord {
	int damage;
	int hitgroup;
	DWORD time;
	bool isKill;
};

static constexpr int MAX_HITS = 16;
static HitRecord hits[MAX_HITS];
static int hitCount = 0;

class HitmarkerListener : public IGameEventListener2
{
public:
	void FireGameEvent(IGameEvent* event) override
	{
		if (!event || !cfg.visuals.hitmarker.Enabled)
			return;

		if (!globals::g_interfaces.Engine->IsInGame())
			return;

		int localIdx = globals::g_interfaces.Engine->GetLocalPlayerIdx();
		int attackerIdx = globals::g_interfaces.Engine->GetPlayerForUserID(event->GetInt("attacker"));

		if (attackerIdx != localIdx)
			return;

		// Don't record self-damage
		int victimIdx = globals::g_interfaces.Engine->GetPlayerForUserID(event->GetInt("userid"));
		if (victimIdx == localIdx)
			return;

		int damage = event->GetInt("dmg_health");
		int hitgroup = event->GetInt("hitgroup");
		int healthRemaining = event->GetInt("health");

		// Store hit record
		if (hitCount < MAX_HITS) {
			hits[hitCount++] = { damage, hitgroup, GetTickCount(), healthRemaining == 0 };
		} else {
			// Shift array and add at end
			for (int i = 0; i < MAX_HITS - 1; i++)
				hits[i] = hits[i + 1];
			hits[MAX_HITS - 1] = { damage, hitgroup, GetTickCount(), healthRemaining == 0 };
		}

		// Play hit sound via engine command
		if (cfg.visuals.hitmarker.Sound) {
			globals::g_interfaces.Engine->ClientCmdUnrestricted(
				healthRemaining == 0 ? "play buttons/arena_switch_press_02" : "buttons/arena_switch_press_02"
			);
		}
	}

	int GetEventDebugID() override { return 42; }
};

static HitmarkerListener listener;
static bool registered = false;

void hitmarker::Init()
{
	if (registered)
		return;

	if (!globals::g_interfaces.GameEventMgr) {
		CrashLog::Write("[Hitmarker] GameEventManager is null - hitmarker unavailable");
		return;
	}

	__try {
		globals::g_interfaces.GameEventMgr->AddListener(&listener, "player_hurt", false);
		registered = true;
		CrashLog::Write("[Hitmarker] Listener registered");
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		CrashLog::Write("[Hitmarker] AddListener crashed - hitmarker unavailable");
		registered = false;
	}
}

void hitmarker::Shutdown()
{
	if (!registered || !globals::g_interfaces.GameEventMgr)
		return;

	__try {
		globals::g_interfaces.GameEventMgr->RemoveListener(&listener);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		CrashLog::Write("[Hitmarker] RemoveListener crashed");
	}
	registered = false;
	hitCount = 0;
}

void hitmarker::Render()
{
	if (!cfg.visuals.hitmarker.Enabled || hitCount == 0)
		return;

	auto DispSize = ImGui::GetIO().DisplaySize;
	float cx = DispSize.x / 2.f;
	float cy = DispSize.y / 2.f;
	auto* dl = ImGui::GetBackgroundDrawList();

	DWORD now = GetTickCount();
	float duration = (float)cfg.visuals.hitmarker.Duration;

	// Render each hit and remove expired ones
	int writeIdx = 0;
	for (int i = 0; i < hitCount; i++)
	{
		float elapsed = (float)(now - hits[i].time);
		if (elapsed > duration) continue; // expired

		float alpha = 1.f - (elapsed / duration);
		float t = elapsed / duration; // 0→1 animation progress

		// Move to write position (compact expired entries)
		if (writeIdx != i)
			hits[writeIdx] = hits[i];
		writeIdx++;

		bool isHeadshot = hits[i].hitgroup == HITGROUP_HEAD;
		bool isKill = hits[i].isKill;

		// Hitmarker color
		auto& baseCol = isKill ? cfg.visuals.hitmarker.KillColor
			: (isHeadshot ? cfg.visuals.hitmarker.HeadshotColor : cfg.visuals.hitmarker.Color);
		ImU32 col = ImColor(baseCol.r, baseCol.g, baseCol.b, baseCol.a * alpha);

		// Draw crosshair lines (4 lines at 45 degrees)
		float lineLen = cfg.visuals.hitmarker.Size;
		float gap = cfg.visuals.hitmarker.Gap;

		// Scale up slightly for kills
		if (isKill) {
			lineLen *= 1.3f;
		}

		for (int corner = 0; corner < 4; corner++)
		{
			float angle = (45.f + 90.f * corner) * (3.14159265f / 180.f);
			float cosA = cosf(angle);
			float sinA = sinf(angle);

			ImVec2 p1(cx + cosA * gap, cy + sinA * gap);
			ImVec2 p2(cx + cosA * (gap + lineLen), cy + sinA * (gap + lineLen));
			dl->AddLine(p1, p2, col, cfg.visuals.hitmarker.Thickness);
		}

		// Draw damage number floating upward
		if (cfg.visuals.hitmarker.ShowDamage)
		{
			float yOffset = -20.f - t * 30.f; // float upward
			auto dmgStr = std::to_string(hits[i].damage);
			ImVec2 txtSize = ImGui::CalcTextSize(dmgStr.c_str());
			ImU32 dmgCol = col; // same color with fading alpha

			// Outlined damage text
			float tx = cx - txtSize.x / 2.f;
			float ty = cy + yOffset;
			dl->AddText(ImVec2(tx + 1, ty + 1), ImColor(0.f, 0.f, 0.f, alpha * 0.8f), dmgStr.c_str());
			dl->AddText(ImVec2(tx, ty), dmgCol, dmgStr.c_str());
		}
	}
	hitCount = writeIdx;
}
