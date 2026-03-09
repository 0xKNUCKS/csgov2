#include "UserCmd.h"

uint32_t CUserCmd::ComputeCRC() const noexcept
{
	SetupCRC crc;

	crc.ProcessBuffer(&command_number, sizeof(command_number));
	crc.ProcessBuffer(&tick_count, sizeof(tick_count));
	crc.ProcessBuffer(&viewangles, sizeof(viewangles));
	crc.ProcessBuffer(&aimdirection, sizeof(aimdirection));
	crc.ProcessBuffer(&forwardmove, sizeof(forwardmove));
	crc.ProcessBuffer(&sidemove, sizeof(sidemove));
	crc.ProcessBuffer(&upmove, sizeof(upmove));
	crc.ProcessBuffer(&buttons, sizeof(buttons));
	crc.ProcessBuffer(&impulse, sizeof(impulse));
	crc.ProcessBuffer(&weaponselect, sizeof(weaponselect));
	crc.ProcessBuffer(&weaponsubtype, sizeof(weaponsubtype));
	crc.ProcessBuffer(&random_seed, sizeof(random_seed));
	crc.ProcessBuffer(&mousedx, sizeof(mousedx));
	crc.ProcessBuffer(&mousedy, sizeof(mousedy));

	return crc;
}