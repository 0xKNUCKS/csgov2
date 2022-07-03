// New Start for my csgo project :))
#include <iostream>
#include "hook.h"


#define RGBA(r,g,b,a)\
D3DCOLOR_RGBA(r,g,b,a)
#define RGB(r,g,b)\
 D3DCOLOR_RGBA(r,g,b,255)

bool Freeze = false;

#define out(c, m, ...) \
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c); \
        printf(m); \
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

void mb(std::string m, std::string t) { MessageBox(0, m.c_str(), t.c_str(), MB_OK); }
bool bLineESP = false;

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

    //AllocConsole();
    //FILE* f;
    //freopen_s(&f, "CONOUT$", "r", stdout);
    //SetConsoleTitleA("DBG con");
    //if (f)
    //    std::cout << "Allocated a Console!";

    //MessageBox(0, "Starting", "Start...", 0);   

    if (gui::Setup())
    {
        if (hooks::Setup())
            return TRUE;
    
        hooks::Destroy();
        gui::Destroy();
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }
    else
    {
        gui::Destroy();
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
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
        CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), hModule, 0, nullptr);
    }
    return TRUE;
}
