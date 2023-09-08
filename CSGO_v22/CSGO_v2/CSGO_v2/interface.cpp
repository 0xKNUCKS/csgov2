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
    ClientEntity = (IClientEntityList*)FindInterface(DLL_CLIENT, "VClientEntityList003");
    BaseClient = (IBaseClientDLL*)FindInterface(DLL_CLIENT, "VClient018");
    Engine = (IVEngineClient*)FindInterface(DLL_ENGINE, "VEngineClient014");
    InputSystem = (CInputSystem*)FindInterface("inputsystem.dll", "InputSystemVersion001");
    ModelInfo = (IVModelInfo*)FindInterface(DLL_ENGINE, "VModelInfoClient004");
    EngineTrace = (IEngineTrace*)FindInterface(DLL_ENGINE, "EngineTraceClient004");
    Cvar        = (ICvar*)FindInterface("vstdlib.dll", "VEngineCvar007");
    Surface     = (ISurface*)FindInterface("vguimatsurface.dll", "VGUI_Surface031");
}