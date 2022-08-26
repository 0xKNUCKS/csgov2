#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>
#include <algorithm>

#include "Process.h"
#include "color.hpp"
#include "utils.h"

// Process and DLL info.
#define PROC_NAME "csgo.exe"
#define WIND_NAME "Counter-Strike: Global Offensive - Direct3D 9"
#define CLSS_NAME "Valve001"
#define DLL_PATH  "E:\\Projects\\C++\\CSGO v3\\Release\\CSGO_v3.dll"
               // "E:\\Projects\\C++\\CSGO v3\\Release\\CSGO_v3.dll"

// a Y/N choice function.
bool choice(const char* str, ...)
{
    char buf;
    std::cout << str;
    std::cin >> buf;
    return ::tolower(buf) == 'y' ? true : false;
}

// CSGO process class
proc_t csgo(PROC_NAME, DLL_PATH, WIND_NAME, CLSS_NAME);
bool insecure = false;

// updates every 100ms
void update()
{
    while (true)
    {
        csgo.hwnd = FindWindow(csgo.className, csgo.windowName);

#ifdef _DEBUG
        if (!csgo.isActive()) { printf(" - "); }; // debug purposes
#endif // _DEBUG


        Sleep(100);
    }
}

// Main function
int main(void)
{
    SetConsoleTitleA("cockbalt.solutions - (Build: " __DATE__ " - " __TIME__ ")");

    // Choose to start CSGO with -insecure or without.
    insecure = choice("(recommended) Start %s with '-insecure'? y/n: ", csgo.Name);

    std::thread thUpdate(update);
    Sleep(200);

    while (true)
    {
        system("cls");
        utils::ascii_art("cockbalt");
        std::cout << "\nWelcome to ad.m CSGO Loader.\njust a simple loadlibrary injector for my CSGO Project.\n\n";

        if (!csgo.isActive())
        {
            std::cout << dye::aqua("\nLaunching CSGO...\n");

            //WinExec("cmd.exe /c powershell.exe Start-Process -FilePath 'C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\csgo.exe' -ArgumentList '-steam -insecure'", SW_HIDE);
            {
                // this will always launch CSGO with the ID 730, to get your game's ID check steam.inf in your game folder and grab it from there <3.
                const char* cmd = insecure ? "\"C:\\Program Files (x86)\\Steam\\steam.exe\" -applaunch 730 -insecure" : "\"C:\\Program Files (x86)\\Steam\\steam.exe\" -applaunch 730";
                system(cmd);
            }

            do { Sleep(50); } while (!csgo.isActive());
        }

        if (!Process::GetProcID(csgo))
        {
            std::cout << dye::red("Getting Process ID Failed! Exiting in 3s...");
            Sleep(3000);
            ExitProcess(0);
        }

        std::cout << dye::green("injecting...\n");

        if (!Process::inject(csgo)) // todo: add dll file exists check
        {
            std::cout << dye::red("injection Failed! Exiting in 3s...");
            Sleep(3000);
            ExitProcess(0);
        }

        std::cout << dye::purple("injection Completed!\n") << dye::yellow("info:\n");
        printf("Dll path: %s\nProc Name: %s\nPID: %d", csgo.dllPath, csgo.Name, csgo.pid);

        system("pause>nul");

        Process::Terminate(csgo);
        Sleep(1000);
    }
}