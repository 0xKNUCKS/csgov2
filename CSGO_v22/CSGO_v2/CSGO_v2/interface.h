#pragma once
#pragma warning (disable : 4430) 
#include "classes.h"
#include "Model.h"

// Macros
#define DLL_ENGINE "engine.dll"
#define DLL_CLIENT "client.dll"

class interfaces_t
{
public:

    interfaces_t()
    {
        interfaces_t::init();
    }

    IClientEntityList* ClientEntity; // = (IClientEntityList*)FindInterface("client.dll", "VClientEntityList003");
    IBaseClientDLL* BaseClient; // = (IBaseClientDLL*)FindInterface("client.dll", "VClient018");
    IVEngineClient* Engine;
    CInputSystem* InputSystem;
    IVModelInfo* ModelInfo;
    IEngineTrace* EngineTrace;
    ICvar* Cvar;
    void init();

private:
    typedef void* (_cdecl* tCreateInterface)(const char* name, int* returnCode);

    inline void* FindInterface(const char* dllname, const char* interfaceName);
};

//extern interfaces_t g_interfaces;
//std::unique_ptr<const interfaces_t> g_interfaces;