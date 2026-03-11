#include "Misc.h"
#include "SDK/Entity/localplayer.h"
#include "SDK/Globals/Globals.h"
#include "lib/Hooks/hook.h"
#include "imgui.h"

namespace gui { extern bool bOpen; }
#include <cmath>
#include <vector>
#include <string>

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

	// Proportional deceleration: stronger at high speed, gentle near stop
	// Clamp to 450 (engine max), scale by speed for smooth curve
	float mult = cfg.misc.movement.AutoStopSpeed;
	float stopForce = (speed * mult < 450.f) ? speed * mult : 450.f;

	cmd->forwardmove = -std::cos(delta) * stopForce;
	cmd->sidemove = std::sin(delta) * stopForce;
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

// ============================================================
// Shared overlay rendering helpers
// ============================================================

// Per-entry fade state for smooth list animations
struct EntryFade {
	float alpha = 0.f;
	bool wasActive = false;
};

// Smooth alpha towards target using frame-rate independent lerp
static float SmoothAlpha(float current, float target, float speed)
{
	float dt = ImGui::GetIO().DeltaTime;
	float step = dt * speed;
	if (step > 1.f) step = 1.f;
	float result = current + (target - current) * step;
	if (std::abs(result - target) < 0.005f) result = target;
	return result;
}

// Common overlay window style (dark, rounded, semi-transparent)
static constexpr ImGuiWindowFlags kOverlayFlags =
	ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
	ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize |
	ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus;

static void PushOverlayStyle(float windowAlpha)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 8));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, windowAlpha);
	// Inherits WindowBg, Border, TitleBg from the global menu theme
}

static void PopOverlayStyle()
{
	ImGui::PopStyleVar(4);
}

// Render a colored tag + label row with per-entry fade
static void OverlayRow(const char* tag, ImVec4 tagColor, const char* label, float alpha)
{
	if (alpha < 0.01f) return;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	ImGui::TextColored(tagColor, "[%s]", tag);
	ImGui::SameLine();
	ImGui::TextUnformatted(label);
	ImGui::PopStyleVar();
}

// ============================================================
// Spectator List
// ============================================================

void misc::SpectatorList()
{
	// Window-level fade: smooth show/hide even when toggled or list empties
	static float windowAlpha = 0.f;
	static EntryFade specFades[65] = {}; // per-player slot

	bool shouldShow = cfg.misc.SpectatorList &&
		globals::g_interfaces.Engine->IsInGame() && hooks::GlobalVars && LocalPlayer.Get();

	// Collect spectators
	struct SpectatorInfo { int slot; std::string name; int mode; };
	std::vector<SpectatorInfo> spectators;

	if (shouldShow)
	{
		int localIdx = globals::g_interfaces.Engine->GetLocalPlayerIdx();

		for (int i = 1; i <= hooks::GlobalVars->maxClients; i++)
		{
			if (i == localIdx) continue;

			auto* ent = globals::g_interfaces.ClientEntity->GetClientEntity(i);
			if (!ent || ent->isDormant()) continue;

			int observerMode = *(int*)((uintptr_t)ent + offsets::m_iObserverMode);
			if (observerMode < 4) continue;

			int handle = *(int*)((uintptr_t)ent + offsets::m_hObserverTarget);
			if (!handle) continue;

			int targetIdx = handle & 0xFFF;
			if (targetIdx != localIdx) continue;

			player_info_s pinfo;
			if (globals::g_interfaces.Engine->getPlayerInfo(i, pinfo))
				spectators.push_back({ i, pinfo.name, observerMode });
		}
	}

	bool hasContent = !spectators.empty();
	// Show title bar when menu is open (so user can reposition), hide when closed and empty
	bool showWindow = shouldShow && (hasContent || gui::bOpen);
	windowAlpha = SmoothAlpha(windowAlpha, showWindow ? 1.f : 0.f, 8.f);

	// Update per-entry fades
	for (int i = 0; i < 65; i++)
	{
		bool isActive = false;
		for (auto& s : spectators)
			if (s.slot == i) { isActive = true; break; }
		specFades[i].alpha = SmoothAlpha(specFades[i].alpha, isActive ? 1.f : 0.f, 10.f);
		specFades[i].wasActive = isActive;
	}

	if (windowAlpha < 0.01f) return;

	ImGui::SetNextWindowSize(ImVec2(170, 0), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 180, 10), ImGuiCond_FirstUseEver);

	PushOverlayStyle(windowAlpha);
	if (ImGui::Begin("Spectators##overlay", nullptr, kOverlayFlags))
	{
		for (auto& spec : spectators)
		{
			const char* tag = "?";
			ImVec4 color(0.7f, 0.7f, 0.7f, 1.f);
			switch (spec.mode) {
				case 4: tag = "1st"; color = ImVec4(1.f, 0.4f, 0.4f, 1.f); break;
				case 5: tag = "3rd"; color = ImVec4(1.f, 0.8f, 0.3f, 1.f); break;
				case 6: tag = "Free"; color = ImVec4(0.5f, 0.8f, 1.f, 1.f); break;
			}
			OverlayRow(tag, color, spec.name.c_str(), specFades[spec.slot].alpha);
		}
	}
	ImGui::End();
	PopOverlayStyle();
}

// ============================================================
// Keybind List
// ============================================================

void misc::KeybindList()
{
	static float windowAlpha = 0.f;

	bool shouldShow = cfg.misc.KeybindList &&
		globals::g_interfaces.Engine->IsInGame() && LocalPlayer.Get();

	// Collect active keybinds
	struct BindEntry {
		const char* name;
		const char* modeTag; // "hold", "on", "always"
		ImVec4 color;
		int id; // stable ID for fade tracking
	};
	std::vector<BindEntry> active;

	if (shouldShow)
	{
		// Aimbot key
		if (cfg.aimbot.Enabled && cfg.aimbot.Key.isActive())
		{
			const char* tag = "hold";
			if (cfg.aimbot.Key.mode == HotkeyMode::Toggle) tag = "on";
			else if (cfg.aimbot.Key.mode == HotkeyMode::AlwaysOn) tag = "always";
			active.push_back({ "Aimbot", tag, ImVec4(1.f, 0.4f, 0.4f, 1.f), 0 });
		}

		// Third person
		if (cfg.visuals.misc.ThirdPerson && cfg.visuals.misc.ThirdPersonKey.isActive())
		{
			const char* tag = "hold";
			if (cfg.visuals.misc.ThirdPersonKey.mode == HotkeyMode::Toggle) tag = "on";
			else if (cfg.visuals.misc.ThirdPersonKey.mode == HotkeyMode::AlwaysOn) tag = "always";
			active.push_back({ "Third Person", tag, ImVec4(0.5f, 0.8f, 1.f, 1.f), 1 });
		}

		// Features that are just toggles (always-on style)
		if (cfg.aimbot.StandaloneRCS && cfg.aimbot.RCS)
			active.push_back({ "RCS", "on", ImVec4(1.f, 0.7f, 0.3f, 1.f), 2 });

		if (cfg.aimbot.autoShoot.Enabled)
			active.push_back({ "Auto Shoot", "on", ImVec4(1.f, 0.5f, 0.5f, 1.f), 3 });

		if (cfg.misc.exploits.FakeLag)
			active.push_back({ "Fake Lag", "on", ImVec4(0.8f, 0.5f, 1.f, 1.f), 4 });

		if (cfg.aimbot.backtrack.Enabled)
			active.push_back({ "Backtrack", "on", ImVec4(0.4f, 1.f, 0.7f, 1.f), 5 });

		if (cfg.aimbot.Silent)
			active.push_back({ "Silent Aim", "on", ImVec4(1.f, 0.6f, 0.8f, 1.f), 6 });
	}

	bool hasContent = !active.empty();
	// Show title bar when menu is open (so user can reposition), hide when closed and empty
	bool showWindow = shouldShow && (hasContent || gui::bOpen);
	windowAlpha = SmoothAlpha(windowAlpha, showWindow ? 1.f : 0.f, 8.f);

	// Per-entry fades (up to 16 bind slots)
	static float bindFades[16] = {};
	for (int i = 0; i < 16; i++)
	{
		bool isActive = false;
		for (auto& b : active)
			if (b.id == i) { isActive = true; break; }
		bindFades[i] = SmoothAlpha(bindFades[i], isActive ? 1.f : 0.f, 10.f);
	}

	if (windowAlpha < 0.01f) return;

	ImGui::SetNextWindowSize(ImVec2(160, 0), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

	PushOverlayStyle(windowAlpha);
	if (ImGui::Begin("Keybinds##overlay", nullptr, kOverlayFlags))
	{
		for (auto& bind : active)
		{
			float alpha = (bind.id < 16) ? bindFades[bind.id] : 1.f;
			OverlayRow(bind.modeTag, bind.color, bind.name, alpha);
		}
	}
	ImGui::End();
	PopOverlayStyle();
}
