#include "aimbot.h"
#include "Globals.h"
#include "localplayer.h"
#include "hook.h"
#include <cmath>

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

// Wrap for the aimbot function (new and sexy, less complicated too)
void aimbot::Run(CUserCmd* cmd)
{
	if (!cfg.aimbot.Enabled)
		return;

	if (!GetAsyncKeyState(cfg.aimbot.Key.virtualKey))
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	gEntity* bestTarget = nullptr;
	float bestFov = cfg.aimbot.FOV;
	float bestDistance = FLT_MAX;
	Target.ent = nullptr;

	for (int i = 0; i <= hooks::GlobalVars->maxClients; i++)
	{
		auto ent = globals::g_interfaces.ClientEntity->GetClientEntity(i);

		if (!ent || !ent->isValidState() || ent->isDormant() ||
			(ent->isTeammate() && !cfg.aimbot.FriendlyFire))
			continue;

		math::Vector targetPos = ent->getBonePosFromChache(8); // Assume bone index 8 is the head
		math::Vector localPos = LocalPlayer->getEyePosition();
		math::Vector aimDirection = (targetPos - localPos).normalize();

		math::Vector viewAngles = globals::g_interfaces.Engine->GetViewAngles();
		math::Vector aimAngles = ent->getAimAtAngles();

		float fov = (viewAngles - aimAngles).length2D();
		float distance = (targetPos - localPos).length();

		if (fov < bestFov || (fov == bestFov && distance < bestDistance))
		{
			bestFov = fov;
			bestDistance = distance;
			bestTarget = ent;

			// store it into the target object
			Target.fov = bestFov;
			Target.distance = bestDistance;
			Target.ent = bestTarget;
		}
	}

	if (bestTarget)
	{
		math::Vector aimAngles = bestTarget->getAimAtAngles();

		math::Vector currentAngles = globals::g_interfaces.Engine->GetViewAngles();
		math::Vector delta = aimAngles - currentAngles;
		
		delta.normalize();

		math::Vector finalAngles = currentAngles + delta / cfg.aimbot.Smooth;

		bool isSilent = cfg.aimbot.Silent;
		if (cmd->buttons & cmd->IN_ATTACK)
			cmd->viewangles = isSilent ? aimAngles : finalAngles; // if its silent aim, aim at target directly without any smooth.
		if (!isSilent)
			globals::g_interfaces.Engine->SetViewAngles(finalAngles); // if silent aim isnt on, itll also change engine's viewAngles

		AdjustMovement(cmd, currentAngles, finalAngles);
	}
}
