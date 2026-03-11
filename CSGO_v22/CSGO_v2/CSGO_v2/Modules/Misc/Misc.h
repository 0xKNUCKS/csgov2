#pragma once
#include "SDK/Classes/classes.h"
#include "SDK/Entity/entity.h"
#include "lib/Configs/config.h"

namespace misc
{
	// Set by hook before aimbot runs — true if user was holding IN_ATTACK before aimbot
	inline bool manualAttack = false;

	// Fake lag state (for visual indicator)
	inline math::Vector lastSentOrigin(0.f, 0.f, 0.f);
	inline int chokedTickCount = 0;

	void BunnyHop(CUserCmd* cmd);
	void AutoStop(CUserCmd* cmd);
	void RadarHack();
	void FakeLag(CUserCmd* cmd, bool* bSendPacket);
	void AntiFlash();
	void NightMode();
	void SpectatorList();
	void KeybindList();
};
