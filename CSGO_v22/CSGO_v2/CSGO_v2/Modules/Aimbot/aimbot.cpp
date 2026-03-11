#include "aimbot.h"
#include "Backtrack.h"
#include "SDK/Globals/Globals.h"
#include "SDK/Entity/localplayer.h"
#include "SDK/Classes/EngineTrace/TraceTypes.h"
#include "lib/Hooks/hook.h"
#include "lib/Notify/Notify.h"
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
	// After a crash, pause aimbot for 2 seconds to avoid SEH-unwind spam tanking FPS
	static DWORD crashCooldown = 0;
	if (crashCooldown && GetTickCount() < crashCooldown)
		return;

	__try {
		aimbot::RunInternal(cmd);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		crashCooldown = GetTickCount() + 2000;
		static DWORD lastNotifyTime = 0;
		DWORD now = GetTickCount();
		if (now - lastNotifyTime > 5000) {
			lastNotifyTime = now;
			Notify::Warn("Aimbot recovered from crash — pausing 2s");
		}
	}
}

void aimbot::RunInternal(CUserCmd* cmd)
{
	if (!cfg.aimbot.Enabled && !cfg.aimbot.StandaloneRCS)
		return;

	if (!globals::g_interfaces.Engine->IsInGame() || !hooks::GlobalVars)
		return;

	gEntity* lp = LocalPlayer.Get();
	if (!lp)
		return;

	bool alive = *(int*)((uintptr_t)lp + offsets::deadFlag) == 0;
	if (!alive)
		return;

	// Track shot count for RCS start bullet (always, regardless of aim key)
	bool isShooting = (cmd->buttons & cmd->IN_ATTACK) != 0;
	if (isShooting && !wasShooting)
		shotsFired++;
	else if (isShooting)
		shotsFired++;
	if (!isShooting)
		shotsFired = 0;
	wasShooting = isShooting;

	// --- Find Best Target (only when aimbot is enabled) ---
	bool aimKeyHeld = cfg.aimbot.Enabled && cfg.aimbot.Key.isActive();
	gEntity* bestTarget = nullptr;
	float bestFov = cfg.aimbot.FOV;
	float bestDistance = FLT_MAX;
	math::Vector bestAimAngles = {};
	math::Vector bestBonePos = {};
	Target.ent = nullptr;

	auto viewOffset = *(math::Vector*)((uintptr_t)lp + offsets::m_vecViewOffset);
	const auto& origin = lp->getAbsOrigin();
	math::Vector localPos = { origin.x + viewOffset.x, origin.y + viewOffset.y, origin.z + viewOffset.z };
	math::Vector viewAngles = globals::g_interfaces.Engine->GetViewAngles();

	int boneIndex = GetBoneIndex(cfg.aimbot.AimBone);

	// Compensate view angles for punch when doing FOV comparison
	math::Vector compensatedAngles = viewAngles;
	math::Vector punch = lp->getAimPunch();
	if (cfg.aimbot.RCS) {
		compensatedAngles.x -= punch.x * cfg.aimbot.RCSAmountX;
		compensatedAngles.y -= punch.y * cfg.aimbot.RCSAmountY;
	}

	for (int i = 1; cfg.aimbot.Enabled && i <= hooks::GlobalVars->maxClients; i++)
	{
		auto ent = globals::g_interfaces.ClientEntity->GetClientEntity(i);

		if (!ent || !ent->isValidState() || ent->isDormant() ||
			(ent->isTeammate() && !cfg.aimbot.FriendlyFire))
			continue;

		math::Vector targetPos;
		if (!GetBonePos(ent, boneIndex, targetPos))
			continue;

		if (cfg.aimbot.VisibilityCheck && !IsVisible(localPos, targetPos, lp, ent))
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
			bestBonePos = targetPos;

			Target.fov = bestFov;
			Target.distance = bestDistance;
			Target.ent = bestTarget;
			Target.bonePos = bestBonePos;
		}
	}

	// --- Backtrack: check past positions for a better target ---
	float backtrackSimTime = 0.f;
	if (cfg.aimbot.backtrack.Enabled)
	{
		BacktrackRecord btRecord;
		int btPlayerIdx = 0;

		if (backtrack::GetBestRecord(lp, localPos, compensatedAngles, bestFov, btRecord, btPlayerIdx))
		{
			// Found a past-tick position closer to crosshair than any current-tick target
			auto btEnt = globals::g_interfaces.ClientEntity->GetClientEntity(btPlayerIdx);
			if (btEnt)
			{
				bestTarget = btEnt;
				bestBonePos = btRecord.headPos;
				bestAimAngles = CalcAimAngles(localPos, btRecord.headPos);
				bestFov = (compensatedAngles - bestAimAngles).length2D();
				backtrackSimTime = btRecord.simTime;

				Target.fov = bestFov;
				Target.distance = (btRecord.headPos - localPos).length();
				Target.ent = btEnt;
				Target.bonePos = btRecord.headPos;
			}
		}
	}

	// --- Auto Shoot (works with or without aimkey) ---
	static int autoShotCount = 0;
	if (cfg.aimbot.autoShoot.Enabled && bestTarget)
	{
		// Check raw FOV from compensated view to target (not smoothed)
		float fovToTarget = (compensatedAngles - bestAimAngles).length2D();

		if (fovToTarget < cfg.aimbot.autoShoot.FOV)
		{
			// Delay between shots
			static DWORD lastAutoShotTime = 0;
			DWORD now = GetTickCount();
			bool delayOk = cfg.aimbot.autoShoot.DelayMs <= 0 ||
				(now - lastAutoShotTime >= (DWORD)cfg.aimbot.autoShoot.DelayMs);

			if (delayOk)
			{
				cmd->buttons |= cmd->IN_ATTACK;
				autoShotCount++;
				// Silent aim to exact target for clean hit reg
				math::Vector fireAngles = bestAimAngles;
				if (cfg.aimbot.RCS && autoShotCount >= cfg.aimbot.RCSStartBullet) {
					fireAngles.x -= punch.x * cfg.aimbot.RCSAmountX;
					fireAngles.y -= punch.y * cfg.aimbot.RCSAmountY;
				}
				cmd->viewangles = fireAngles;
				AdjustMovement(cmd, viewAngles, fireAngles);
				globals::g_interfaces.Engine->SetViewAngles(viewAngles); // Prevent snap
				lastAutoShotTime = now;
			}
		}
		else {
			autoShotCount = 0; // Reset when target leaves FOV
		}
	}
	else {
		autoShotCount = 0; // Reset when auto-shoot inactive or no target
	}

	// --- Aimbot Aim (requires aim key) ---
	if (aimKeyHeld && bestTarget)
	{
		// Apply recoil compensation to aim angles
		math::Vector rcsAimAngles = bestAimAngles;
		if (cfg.aimbot.RCS && shotsFired >= cfg.aimbot.RCSStartBullet) {
			rcsAimAngles.x -= punch.x * cfg.aimbot.RCSAmountX;
			rcsAimAngles.y -= punch.y * cfg.aimbot.RCSAmountY;
		}

		math::Vector currentAngles = globals::g_interfaces.Engine->GetViewAngles();
		math::Vector delta = rcsAimAngles - currentAngles;

		delta.normalizeDeg();
		delta.normalize();

		float smoothX = cfg.aimbot.Smooth * cfg.aimbot.SmoothX;
		float smoothY = cfg.aimbot.Smooth * cfg.aimbot.SmoothY;

		math::Vector finalAngles;
		finalAngles.x = currentAngles.x + delta.x / smoothX;
		finalAngles.y = currentAngles.y + delta.y / smoothY;
		finalAngles.z = 0.f;

		bool isSilent = cfg.aimbot.Silent;

		if (cmd->buttons & cmd->IN_ATTACK)
			cmd->viewangles = isSilent ? rcsAimAngles : finalAngles;
		if (!isSilent)
			globals::g_interfaces.Engine->SetViewAngles(finalAngles);
		else
			globals::g_interfaces.Engine->SetViewAngles(viewAngles); // Prevent snap when target dies

		AdjustMovement(cmd, currentAngles, isSilent ? viewAngles : finalAngles);
	}

	// --- Standalone RCS (compensate recoil while shooting, independent of aimbot) ---
	if (!aimKeyHeld && cfg.aimbot.StandaloneRCS) {
		// Track total compensation applied to the view so far (absolute, not per-frame delta)
		static math::Vector totalApplied(0.f, 0.f, 0.f);

		bool shooting = (cmd->buttons & cmd->IN_ATTACK) && shotsFired >= cfg.aimbot.RCSStartBullet;
		math::Vector punch = lp->getAimPunch();

		// Target = how much total the view should be shifted right now
		math::Vector target;
		target.x = punch.x * cfg.aimbot.RCSAmountX;
		target.y = punch.y * cfg.aimbot.RCSAmountY;

		// Punch has fully decayed and we've unwound — clean reset
		if (!shooting && std::abs(punch.x) < 0.01f && std::abs(punch.y) < 0.01f) {
			totalApplied.x = 0.f;
			totalApplied.y = 0.f;
			totalApplied.z = 0.f;
		}
		// Actively shooting OR still unwinding previous compensation as punch decays
		else if (shooting || std::abs(totalApplied.x) > 0.01f || std::abs(totalApplied.y) > 0.01f) {
			math::Vector delta;
			delta.x = target.x - totalApplied.x;
			delta.y = target.y - totalApplied.y;

			// Apply RCS smoothing (exponential convergence — always catches up)
			if (cfg.aimbot.RCSSmooth > 1.0f) {
				delta.x /= cfg.aimbot.RCSSmooth;
				delta.y /= cfg.aimbot.RCSSmooth;
			}

			math::Vector angles = globals::g_interfaces.Engine->GetViewAngles();
			angles.x -= delta.x;
			angles.y -= delta.y;
			angles.normalize();

			if (cfg.aimbot.SilentRCS) {
				// Silent: bypass incremental tracking, directly compensate on cmd only
				math::Vector silentAngles = viewAngles;
				silentAngles.x -= target.x;
				silentAngles.y -= target.y;
				silentAngles.normalize();
				cmd->viewangles = silentAngles;
				globals::g_interfaces.Engine->SetViewAngles(viewAngles);
				// Keep totalApplied in sync so unwind works if toggled off
				totalApplied = target;
			} else {
				globals::g_interfaces.Engine->SetViewAngles(angles);
				cmd->viewangles = angles;
				totalApplied.x += delta.x;
				totalApplied.y += delta.y;
			}
		}
	}
}
