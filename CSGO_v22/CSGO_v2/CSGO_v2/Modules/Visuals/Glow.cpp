#include "Glow.h"
#include "SDK/Globals/Globals.h"
#include "SDK/Entity/localplayer.h"
#include "lib/Hooks/hook.h"
#include "lib/Configs/config.h"
#include "lib/utils/utils.h"
#include "lib/Error/CrashLog.h"

bool glow::Init()
{
	HMODULE clientDll = GetModuleHandleA("client.dll");
	if (!clientDll)
	{
		CrashLog::Write("[Glow] client.dll not found");
		return false;
	}

	// Signature: 0F 11 05 ? ? ? ? 83 C8 01
	auto addr = utils::PatternScan(clientDll, "0F 11 05 ? ? ? ? 83 C8 01");
	if (!addr)
	{
		CrashLog::Write("[Glow] GlowObjectManager pattern not found");
		return false;
	}

	// addr + 3 points to the 4-byte absolute address operand
	uintptr_t absAddr = *reinterpret_cast<uintptr_t*>(addr + 3);
	glowManager = reinterpret_cast<GlowObjectManager_t*>(absAddr);

	if (!glowManager)
	{
		CrashLog::Write("[Glow] GlowObjectManager pointer is null");
		return false;
	}

	CrashLog::Writef("[Glow] GlowObjectManager at 0x%p", glowManager);
	return true;
}

// SEH-safe: clear all glow objects to force a state transition after reinjection
static void ResetGlow()
{
	__try {
		int size = glow::glowManager->glowObjects.size;
		if (size <= 0 || size > 4096)
			return;

		for (int i = 0; i < size; i++)
		{
			auto& obj = glow::glowManager->glowObjects[i];
			if (obj.isUnused())
				continue;
			obj.renderWhenOccluded = false;
			obj.renderWhenUnoccluded = false;
			obj.a = 0.f;
			obj.glowAlphaMax = 0.f;
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {}
}

// SEH-safe glow application (no C++ objects with destructors)
static void ApplyGlow(gEntity* lp)
{
	__try {
		int size = glow::glowManager->glowObjects.size;
		if (size <= 0 || size > 4096)
			return;

		for (int i = 0; i < size; i++)
		{
			auto& obj = glow::glowManager->glowObjects[i];

			if (obj.isUnused() || !obj.pEntity)
				continue;

			gEntity* ent = obj.pEntity;

			__try {
				if (!ent->isPlayer() || !ent->isAlive() || ent->isDormant())
					continue;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				continue;
			}

			float intensity = cfg.visuals.glow.Intensity;

			bool sync = cfg.visuals.glow.SyncWithChams;

			if (ent == lp)
			{
				if (cfg.visuals.glow.LocalPlayer)
				{
					auto& c = sync ? cfg.visuals.chams.LocalVisibleColor : cfg.visuals.glow.LocalColor;
					obj.r = c.r;
					obj.g = c.g;
					obj.b = c.b;
					obj.a = c.a * intensity;
					obj.renderWhenOccluded = true;
					obj.renderWhenUnoccluded = sync;
					obj.glowAlphaMax = intensity;
					obj.glowStyle = cfg.visuals.glow.Style;
				}
				continue;
			}

			bool isTeammate = ent->isTeammate();

			if (isTeammate && !cfg.visuals.glow.Friendly)
				continue;

			const CfgColor* col;
			if (sync)
				col = isTeammate ? &cfg.visuals.chams.FriendlyVisibleColor : &cfg.visuals.chams.EnemyVisibleColor;
			else
				col = isTeammate ? &cfg.visuals.glow.FriendlyColor : &cfg.visuals.glow.EnemyColor;
			auto& c = *col;
			obj.r = c.r;
			obj.g = c.g;
			obj.b = c.b;
			obj.a = c.a * intensity;
			obj.renderWhenOccluded = true;
			obj.renderWhenUnoccluded = sync;
			obj.glowAlphaMax = intensity;
			obj.glowStyle = cfg.visuals.glow.Style;
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		// GlowObjectManager state changed, skip
	}
}

void glow::Run()
{
	if (!cfg.visuals.glow.Enabled)
		return;

	if (!glowManager)
		return;

	if (!globals::g_interfaces.Engine->IsInGame() || !hooks::GlobalVars)
		return;

	gEntity* lp = LocalPlayer.Get();
	if (!lp)
		return;

	// After (re)injection, clear stale glow state for one frame to force a state transition
	static bool needsReset = true;
	if (needsReset) {
		needsReset = false;
		ResetGlow();
		return; // Skip one frame so the engine processes the cleared state
	}

	ApplyGlow(lp);
}
