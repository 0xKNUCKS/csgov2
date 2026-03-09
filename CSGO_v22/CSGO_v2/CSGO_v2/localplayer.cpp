#include "localplayer.h"
#include "Globals.h"

gEntity* localplayer_t::Get()
{
	if (!globals::g_interfaces.Engine || !globals::g_interfaces.ClientEntity)
		return nullptr;

	int localIdx = globals::g_interfaces.Engine->GetLocalPlayerIdx();
	return globals::g_interfaces.ClientEntity->GetClientEntity(localIdx);
}
