#pragma once
#pragma warning (disable : 4430) 
#include "SDK/Classes/classes.h"
#include "SDK/Models/Model.h"

// Macros
#define DLL_ENGINE "engine.dll"
#define DLL_CLIENT "client.dll"

class interfaces_t
{
public:

    IClientEntityList* ClientEntity = nullptr;
    IBaseClientDLL* BaseClient = nullptr;
    IVEngineClient* Engine = nullptr;
    CInputSystem* InputSystem = nullptr;
    IVModelInfo* ModelInfo = nullptr;
    IEngineTrace* EngineTrace = nullptr;
    ICvar* Cvar = nullptr;
    ISurface* Surface = nullptr;

    bool init();

private:
    template <typename retType>
    inline retType* FindInterface(const char* dllname, const char* interfaceName);
};

//extern interfaces_t g_interfaces;
//std::unique_ptr<const interfaces_t> g_interfaces;