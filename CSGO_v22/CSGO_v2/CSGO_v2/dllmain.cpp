// New Start for my csgo project :))
#include <iostream>
#include <Windows.h>
#include "lib/Hooks/hook.h"
#include "lib/Hooks/GUI/GUI.h"
#include "SDK/Globals/Globals.h"
#include "lib/utils/utils.h"
#include "lib/Configs/config.h"
#include "lib/Error/Log.h"

/// Macros
// Hash sum of game files to confirm the game's version
#define GAME_HASHSUM "de96421a66fa72eed00b6edefcf1948a" // ver: 1566

// Main thread
DWORD WINAPI Main(HMODULE hModule)
{
    Log::Info("Main", "Waiting for serverbrowser.dll...");
    while (!GetModuleHandle("serverbrowser.dll")) { Sleep(100); }
    Log::Info("Main", "serverbrowser.dll loaded, starting initialization");

    // Store hModule so the unload thread can call FreeLibraryAndExitThread
    hooks::hModule = hModule;

    // Initialize game interfaces and netvars now that all game modules are loaded
    if (!globals::g_interfaces.init()) {
        Log::Fatal("Main", "Interface initialization failed — aborting");
        Log::DumpToFile("csgo_v2_errors.log");
        FreeLibraryAndExitThread(hModule, 0);
        return FALSE;
    }

    if (!globals::g_NetVars.Init()) {
        Log::Err("Main", "Some netvars failed to resolve — continuing with reduced features");
        // Don't abort — netvars failing is not fatal, features that use them will just not work
    }

    if (!gui::Setup()) {
        Log::Fatal("Main", "GUI/DirectX setup failed — aborting");
        Log::DumpToFile("csgo_v2_errors.log");
        gui::Destroy();
        FreeLibraryAndExitThread(hModule, 0);
        return FALSE;
    }

    if (!hooks::Setup()) {
        Log::Fatal("Main", "Hook setup failed — aborting");
        Log::DumpToFile("csgo_v2_errors.log");
        hooks::Destroy();
        gui::Destroy();
        FreeLibraryAndExitThread(hModule, 0);
        return FALSE;
    }

    Log::Info("Main", "Initialization complete — {} warnings, {} errors",
        Log::Count(Error::Severity::Warning), Log::Count(Error::Severity::Error));

    // Dump any warnings to log file even on success
    if (Log::Count(Error::Severity::Warning) > 0)
        Log::DumpToFile("csgo_v2_errors.log");

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
        utils::SetupConsole();
        CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), hModule, 0, nullptr);
#else
        if (utils::CheckVersion(GAME_HASHSUM)) {
            CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), hModule, 0, nullptr);
        }
#endif
    }
    return TRUE;
}
