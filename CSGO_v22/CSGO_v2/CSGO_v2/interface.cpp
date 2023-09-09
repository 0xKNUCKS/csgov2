#include "interface.h"

template <typename retType>
retType* interfaces_t::FindInterface(const char* dllname, const char* interfaceName)
{
    HMODULE Module = GetModuleHandleA(dllname);
    if (!Module)
        return nullptr;

    typedef retType* (_cdecl* tCreateInterface)(const char* name, int* returnCode);
    tCreateInterface CreateInterface = (tCreateInterface)GetProcAddress(Module, "CreateInterface");
    if (!CreateInterface)
        return nullptr;

    if (retType* rinterface = CreateInterface(interfaceName, nullptr))
        return rinterface;

    return nullptr;
}

void interfaces_t::init()
{
    ClientEntity =  FindInterface<IClientEntityList>(DLL_CLIENT, "VClientEntityList003");
    BaseClient =    FindInterface<IBaseClientDLL>(DLL_CLIENT, "VClient018");
    Engine =        FindInterface<IVEngineClient>(DLL_ENGINE, "VEngineClient014");
    InputSystem =   FindInterface<CInputSystem>("inputsystem.dll", "InputSystemVersion001");
    ModelInfo =     FindInterface<IVModelInfo>(DLL_ENGINE, "VModelInfoClient004");
    EngineTrace =   FindInterface<IEngineTrace>(DLL_ENGINE, "EngineTraceClient004");
    Cvar =          FindInterface<ICvar>("vstdlib.dll", "VEngineCvar007");
    Surface =       FindInterface<ISurface>("vguimatsurface.dll", "VGUI_Surface031");
}