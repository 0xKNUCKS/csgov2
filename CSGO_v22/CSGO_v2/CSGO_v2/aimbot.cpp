#include "aimbot.h"

void aimbot::Run(CUserCmd* cmd)
{
	if (!cfg.aimbot.Enabled)
		return;

	if (!GetAsyncKeyState(cfg.aimbot.Key))
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	FindBestEnt(cfg.aimbot, cmd);

	if (Target.ent.isTeammate() && !cfg.aimbot.FriendlyFire)
		return;

	if (Target.ent.isValidState())
		cmd->viewangles = Target.ent.GetAimAtAngles().normalize();
}

void aimbot::FindBestEnt(Config::Aimbot cfg, CUserCmd* cmd)
{
	Target.fov = cfg.FOV; // aka "best fov"

	for (int i = 1; i <= globals::g_interfaces.Engine->GetMaxClients(); i++)
	{
		ent_t ent(i);

		if (!ent.isValidState())
			continue;
		
		float fov = abs((cmd->viewangles - ent.GetAimAtAngles()).normalize().length());

		if (fov > cfg.FOV)
			continue;

		if (fov < cfg.FOV)
		{
			Target.fov = fov;
			Target.ent = ent;
		}
	}
}
