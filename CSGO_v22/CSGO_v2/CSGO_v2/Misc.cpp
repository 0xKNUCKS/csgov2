#include "Misc.h"

void misc::BunnyHop(CUserCmd* cmd)
{
	if (!cfg.misc.movement.BunnyHop)
		return;

	if (!LocalPlayer.Get())
		return;

	if ((cmd->buttons & CUserCmd::IN_JUMP) && !((LocalPlayer.Flags() & PlayerFlag_OnGround) || (LocalPlayer.Flags() & PlayerFlag_PartialGround))) {
		cmd->buttons &= ~CUserCmd::IN_JUMP;
	}
}
