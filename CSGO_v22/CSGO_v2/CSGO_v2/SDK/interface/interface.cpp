#include "interface.h"
#include "lib/Error/Log.h"

template <typename retType>
retType* interfaces_t::FindInterface(const char* dllname, const char* interfaceName)
{
    HMODULE Module = GetModuleHandleA(dllname);
    if (!Module) {
        Log::Fatal("Interfaces", "GetModuleHandle failed for '{}'", dllname);
        return nullptr;
    }

    typedef retType* (_cdecl* tCreateInterface)(const char* name, int* returnCode);
    tCreateInterface CreateInterface = (tCreateInterface)GetProcAddress(Module, "CreateInterface");
    if (!CreateInterface) {
        Log::Fatal("Interfaces", "GetProcAddress('CreateInterface') failed in '{}'", dllname);
        return nullptr;
    }

    if (retType* rinterface = CreateInterface(interfaceName, nullptr)) {
        Log::Info("Interfaces", "Captured '{}' from '{}' at {:#x}",
            interfaceName, dllname, (uintptr_t)rinterface);
        return rinterface;
    }

    Log::Fatal("Interfaces", "CreateInterface('{}') returned null from '{}'", interfaceName, dllname);
    return nullptr;
}

bool interfaces_t::init()
{
    bool allOk = true;

    ClientEntity =  FindInterface<IClientEntityList>(DLL_CLIENT, "VClientEntityList003");
    BaseClient =    FindInterface<IBaseClientDLL>(DLL_CLIENT, "VClient018");
    Engine =        FindInterface<IVEngineClient>(DLL_ENGINE, "VEngineClient014");
    InputSystem =   FindInterface<CInputSystem>("inputsystem.dll", "InputSystemVersion001");
    ModelInfo =     FindInterface<IVModelInfo>(DLL_ENGINE, "VModelInfoClient004");
    EngineTrace =   FindInterface<IEngineTrace>(DLL_ENGINE, "EngineTraceClient004");
    Cvar =          FindInterface<ICvar>("vstdlib.dll", "VEngineCvar007");
    Surface =       FindInterface<ISurface>("vguimatsurface.dll", "VGUI_Surface031");

    // Validate critical interfaces - these are required for the mod to function
    struct { void* ptr; const char* name; } required[] = {
        { ClientEntity, "ClientEntity" },
        { BaseClient,   "BaseClient" },
        { Engine,       "Engine" },
        { ModelInfo,    "ModelInfo" },
        { Cvar,         "Cvar" },
        { Surface,      "Surface" },
    };

    for (const auto& iface : required) {
        if (!iface.ptr) {
            Log::Fatal("Interfaces", "Critical interface '{}' is null - cannot continue", iface.name);
            allOk = false;
        }
    }

    // Non-critical: warn but don't fail
    if (!InputSystem)
        Log::Warn("Interfaces", "InputSystem is null - some features may not work");
    if (!EngineTrace)
        Log::Warn("Interfaces", "EngineTrace is null - trace features unavailable");

    if (allOk)
        Log::Info("Interfaces", "All interfaces captured successfully");

    return allOk;
}
