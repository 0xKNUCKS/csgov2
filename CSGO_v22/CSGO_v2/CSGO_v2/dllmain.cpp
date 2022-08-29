// New Start for my csgo project :))
#include <iostream>
#include "hook.h"

    // %c = character
    // %s = string (array of characters) 
    // %f = float
    // %lf = double
    // %d = integer
    // %.1 = decimal precision
    // %1 = minimum field width
    // %- = left align  

DWORD WINAPI Main(HMODULE hModule)
{
    while (!GetModuleHandle("serverbrowser.dll")) { Sleep(100); }

    if (gui::Setup())
    {
        if (hooks::Setup()) {
            utils::SetupConsole();
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

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        if (utils::CheckVersion("54f789490156e0d18aa48671c7180431")) {
            CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), hModule, 0, nullptr);
        }
    }
    return TRUE;
}
