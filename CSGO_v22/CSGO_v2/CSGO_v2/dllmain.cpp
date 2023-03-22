// New Start for my csgo project :))
#include <iostream>
#include "hook.h"

// Macros
#define GAME_HASHSUM "846b9223bed47816671d150e2aa93113"

// Main thread
DWORD WINAPI Main(HMODULE hModule)
{
    while (!GetModuleHandle("serverbrowser.dll")) { Sleep(100); }

    if (gui::Setup())
    {
        if (hooks::Setup()) {
            //utils::SetupConsole();
            return TRUE;
        }

        hooks::Destroy();
        gui::Destroy();
        FreeLibraryAndExitThread(hModule, 0);
    }
    else
    {
        gui::Destroy();
        FreeLibraryAndExitThread(hModule, 0);
    }

    return TRUE;
}

// DllMain (main func)
BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
#ifdef _DEBUG
        CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), hModule, 0, nullptr);
#else
        if (utils::CheckVersion(GAME_HASHSUM)) {
            CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), hModule, 0, nullptr);
        }
#endif
    }
    return TRUE;
}
