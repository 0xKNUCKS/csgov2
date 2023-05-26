#include "Misc.h"

void misc::BunnyHop(CUserCmd* cmd)
{
	if (!cfg.misc.movement.BunnyHop)	
		return;

	if (!LocalPlayer.Get())
		return;

	if ((cmd->buttons & CUserCmd::IN_JUMP)) {
		// ON AIR
		if (!(LocalPlayer.Flags() & PlayerFlag_OnGround))
		{
			cmd->buttons &= ~CUserCmd::IN_JUMP;

			if (cfg.misc.movement.AirDuck)
				cmd->buttons |= CUserCmd::IN_DUCK;

			if (cfg.misc.movement.Strafe && !((cmd->buttons & CUserCmd::IN_MOVELEFT) || (cmd->buttons & CUserCmd::IN_MOVERIGHT)))
			{
				// the most basic auto strafe u could make lol
				if (ABS(cmd->mousedx) >= 2) {
					cmd->sidemove = cmd->mousedx < 0 ? -450.f : 450.f;
				}
				else
				{
					// when going forward Implement fast flipping in directions like seen in csgo's movment tricks to gain speed even faster and dont stop the speed flow too.
					static bool Direction;
					static auto lastTick = cmd->tick_count;
					if (cmd->tick_count != lastTick) // so we dont do it more than once in ONE tick
					{
						if (Direction) {
							cmd->viewangles = (cmd->viewangles + math::Vector(0, 1, 0)).normalize();
							cmd->sidemove = 450.f;
						}
						else
						{
							cmd->viewangles = (cmd->viewangles - math::Vector(0, 1, 0)).normalize();
							cmd->sidemove = -450.f;
						}

						Direction = !Direction;
					}

					lastTick = cmd->tick_count;
				}
			}
		}
	}
}
