#include "interface.h"

void* interfaces_t::FindInterface(const char* dllname, const char* interfaceName)
{

    if (tCreateInterface CreateInterface = (tCreateInterface)GetProcAddress(GetModuleHandleA(dllname), "CreateInterface"))
    {
        if (void* rinterface = CreateInterface(interfaceName, nullptr))
            return rinterface;
    }

    return 0;
}

void interfaces_t::init()
{
    ClientEntity = (IClientEntityList*)FindInterface("client.dll", "VClientEntityList003");
    BaseClient = (IBaseClientDLL*)FindInterface("client.dll", "VClient018");
    Engine = (IVEngineClient*)FindInterface("engine.dll", "VEngineClient014");
    DebugOverlay = (IVDebugOverlay*)FindInterface("engine.dll", "VDebugOverlay004");
}