#pragma once
#pragma warning (disable : 4430) 
#include "classes.h"

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
    IVDebugOverlay* DebugOverlay;
    CInputSystem* InputSystem;

    void init();

private:
    typedef void* (_cdecl* tCreateInterface)(const char* name, int* returnCode);

    inline void* FindInterface(const char* dllname, const char* interfaceName);
};

//extern interfaces_t g_interfaces;
//std::unique_ptr<const interfaces_t> g_interfaces;