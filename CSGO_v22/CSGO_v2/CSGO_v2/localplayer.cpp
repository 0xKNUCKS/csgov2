#include "localplayer.h"
#include "Globals.h"

gEntity* localplayer_t::Get()
{
	Local = globals::g_interfaces.ClientEntity->GetClientEntity(1);
	return Local;
}
