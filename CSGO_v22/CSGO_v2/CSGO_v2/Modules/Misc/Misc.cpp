#include "Misc.h"
#include "SDK/Entity/localplayer.h"
#include "SDK/Globals/Globals.h"
#include "lib/Hooks/hook.h"
#include <cmath>

// Constants for CS:GO movement
constexpr float AIR_ACCELERATION = 0.025f;
constexpr float MAX_SPEED = 350.0f;

void misc::BunnyHop(CUserCmd* cmd)
{
	if (!cfg.misc.movement.BunnyHop)	
		return;

	if (!LocalPlayer.Get())
		return;

	if ((cmd->buttons & CUserCmd::IN_JUMP)) {
		// ON AIR
		if (!(LocalPlayer->flags() & PlayerFlag_OnGround))
		{
			cmd->buttons &= ~CUserCmd::IN_JUMP;

			if (cfg.misc.movement.AirDuck)
				cmd->buttons |= CUserCmd::IN_DUCK;

			if (cfg.misc.movement.Strafe)
			{
				// Get current velocity
				auto velocity = LocalPlayer->getVelocity();
				float currentSpeed = velocity.length2D();
				
				// Calculate optimal strafe angle based on current speed
				float optimalAngle = 0.0f;
				if (currentSpeed > 0.0f) {
					optimalAngle = std::atan2(velocity.y, velocity.x);
				}

				// Get keyboard input
				bool movingForward = (cmd->buttons & CUserCmd::IN_FORWARD) != 0;
				bool movingBackward = (cmd->buttons & CUserCmd::IN_BACK) != 0;
				bool movingLeft = (cmd->buttons & CUserCmd::IN_MOVELEFT) != 0;
				bool movingRight = (cmd->buttons & CUserCmd::IN_MOVERIGHT) != 0;

				// Determine base direction
				float baseDirection = 0.0f;
				bool usingKeyboardDirection = false;

				// Check if any movement key is pressed
				if (movingForward || movingBackward || movingLeft || movingRight) {
					usingKeyboardDirection = true;
					// Set base direction based on keyboard input
					if (movingForward) {
						if (movingLeft) baseDirection = -45.0f;
						else if (movingRight) baseDirection = 45.0f;
						else baseDirection = 0.0f;
					}
					else if (movingBackward) {
						if (movingLeft) baseDirection = -135.0f;
						else if (movingRight) baseDirection = 135.0f;
						else baseDirection = 180.0f;
					}
					else {
						if (movingLeft) baseDirection = -90.0f;
						else if (movingRight) baseDirection = 90.0f;
					}
				}

				// Advanced strafing logic
				if (ABS(cmd->mousedx) >= 2) {
					// Mouse-based strafing with acceleration
					float targetYaw = cmd->viewangles.y;
					if (usingKeyboardDirection) {
						// Use keyboard direction as base
						targetYaw += baseDirection;
					} else {
						// Use mouse direction as base
						targetYaw += (cmd->mousedx < 0 ? -90.0f : 90.0f);
					}
					
					float currentYaw = std::atan2(velocity.y, velocity.x) * 180.0f / PI;
					float deltaYaw = targetYaw - currentYaw;
					
					// Apply acceleration based on speed
					float acceleration = AIR_ACCELERATION;
					float speedFactor = min(currentSpeed / MAX_SPEED, 1.0f);
					float moveSpeed = 450.0f * (1.0f - speedFactor * 0.5f);
					
					// Apply movement based on keyboard input
					if (usingKeyboardDirection) {
						cmd->forwardmove = (movingForward ? moveSpeed : (movingBackward ? -moveSpeed : 0.0f));
						cmd->sidemove = (movingRight ? moveSpeed : (movingLeft ? -moveSpeed : 0.0f));
					} else {
						cmd->sidemove = cmd->mousedx < 0 ? -moveSpeed : moveSpeed;
					}
					
					// Adjust view angles for optimal strafing
					cmd->viewangles.y += deltaYaw * acceleration;
				}
				else {
					// Automatic strafing with optimal angles
					static float lastYaw = 0.0f;
					static auto lastTick = cmd->tick_count;
					
					if (cmd->tick_count != lastTick) {
						float targetYaw = optimalAngle * 180.0f / PI;
						if (usingKeyboardDirection) {
							targetYaw += baseDirection;
						}
						float deltaYaw = targetYaw - lastYaw;
						
						// Apply acceleration based on speed
						float acceleration = AIR_ACCELERATION;
						float speedFactor = min(currentSpeed / MAX_SPEED, 1.0f);
						float moveSpeed = 450.0f * (1.0f - speedFactor * 0.5f);
						
						// Apply movement based on keyboard input
						if (usingKeyboardDirection) {
							cmd->forwardmove = (movingForward ? moveSpeed : (movingBackward ? -moveSpeed : 0.0f));
							cmd->sidemove = (movingRight ? moveSpeed : (movingLeft ? -moveSpeed : 0.0f));
						} else {
							// Use automatic strafing when no keyboard input
							static bool direction = false;
							if (direction) {
								cmd->viewangles.y += deltaYaw * acceleration;
								cmd->sidemove = moveSpeed;
							}
							else {
								cmd->viewangles.y -= deltaYaw * acceleration;
								cmd->sidemove = -moveSpeed;
							}
							direction = !direction;
						}
						
						lastYaw = cmd->viewangles.y;
					}
					
					lastTick = cmd->tick_count;
				}

				// Normalize view angles
				cmd->viewangles.normalize();
			}
		}
	}
}

void misc::AutoStop(CUserCmd* cmd)
{
	if (!cfg.misc.movement.AutoStop)
		return;

	if (!LocalPlayer.Get())
		return;

	// Stop movement whenever IN_ATTACK is set (after aimbot/auto-shoot)
	bool attacking = (cmd->buttons & CUserCmd::IN_ATTACK) != 0;
	if (!attacking)
		return;

	// Mode filtering: 0=All, 1=Manual Only, 2=Auto-Shoot Only
	// misc::manualAttack is set BEFORE aimbot runs — true if user held mouse1
	switch (cfg.misc.movement.AutoStopMode)
	{
	case 1: // Manual Only
		if (!manualAttack) return;
		break;
	case 2: // Auto-Shoot Only
		if (manualAttack) return;
		break;
	default: // 0 = All
		break;
	}

	// Don't stop in air
	if (!(LocalPlayer->flags() & PlayerFlag_OnGround))
		return;

	auto velocity = LocalPlayer->getVelocity();
	float speed = velocity.length2D();

	if (speed < 1.f)
		return;

	// Calculate reverse movement direction relative to view angles
	float direction = std::atan2(velocity.y, velocity.x) * RAD_TO_DEG;
	float viewYaw = cmd->viewangles.y;
	float delta = (direction - viewYaw) * DEG_TO_RAD;

	cmd->forwardmove = -std::cos(delta) * 450.f;
	cmd->sidemove = std::sin(delta) * 450.f;
}

void misc::RadarHack()
{
	if (!cfg.misc.RadarHack)
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	if (!LocalPlayer.Get())
		return;

	for (int i = 1; i <= hooks::GlobalVars->maxClients; i++)
	{
		gEntity* ent = globals::g_interfaces.ClientEntity->GetClientEntity(i);
		if (!ent || ent == LocalPlayer.Get())
			continue;

		if (ent->isDormant())
			continue;

		// Set spotted = true so they appear on the in-game radar
		*(bool*)((uintptr_t)ent + offsets::m_bSpotted) = true;
	}
}

void misc::AntiFlash()
{
	if (!cfg.misc.AntiFlash)
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	if (!LocalPlayer.Get())
		return;

	// Clamp flash alpha so we can still see through flashbangs
	float* flashAlpha = (float*)((uintptr_t)LocalPlayer.Get() + offsets::m_flFlashMaxAlpha);
	if (*flashAlpha > cfg.misc.FlashMaxAlpha)
		*flashAlpha = cfg.misc.FlashMaxAlpha;
}

void misc::NightMode()
{
	static bool lastState = false;
	static float lastBrightness = -1.f;

	bool active = cfg.visuals.misc.NightMode;
	float brightness = cfg.visuals.misc.NightModeBrightness;

	// Only update convar when state or value changes
	if (active == lastState && brightness == lastBrightness)
		return;

	if (!globals::g_interfaces.Cvar)
		return;

	ConVar* cv = globals::g_interfaces.Cvar->FindVar("mat_force_tonemap_scale");
	if (cv)
		cv->SetValue(active ? brightness : 0.f); // 0 = game default

	lastState = active;
	lastBrightness = brightness;
}

void misc::FakeLag(CUserCmd* cmd, bool* bSendPacket)
{
	if (!cfg.misc.exploits.FakeLag)
	{
		chokedTickCount = 0;
		return;
	}

	if (!LocalPlayer.Get())
		return;

	// Don't fake lag when dead
	bool alive = *(int*)((uintptr_t)LocalPlayer.Get() + offsets::deadFlag) == 0;
	if (!alive)
		return;

	// Don't choke while shooting — send packets immediately for accurate hit reg
	if (cmd->buttons & CUserCmd::IN_ATTACK)
	{
		*bSendPacket = true;
		lastSentOrigin = LocalPlayer->getAbsOrigin();
		chokedTickCount = 0;
		return;
	}

	static int chokedTicks = 0;
	int maxChoke = cfg.misc.exploits.FakeLagAmount;
	if (maxChoke < 1) maxChoke = 1;
	if (maxChoke > 14) maxChoke = 14;

	if (chokedTicks < maxChoke) {
		*bSendPacket = false;
		chokedTicks++;
	} else {
		*bSendPacket = true;
		lastSentOrigin = LocalPlayer->getAbsOrigin();
		chokedTicks = 0;
	}

	chokedTickCount = chokedTicks;
}
