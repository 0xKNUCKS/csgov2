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
#define DLL_PATH  "CSGO_v2.dll"
               // "E:\\Projects\\C++\\CSGO v3\\Release\\CSGO_v3.dll"

// CSGO process class
proc_t csgo(PROC_NAME, DLL_PATH, WIND_NAME, CLSS_NAME);
int insecure = -1;

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
    SetConsoleTitleA("cockbalt.solutions - (Build: " __DATE__ ")");

    std::thread thUpdate(update);
    Sleep(200);

    while (true)
    {
        system("cls");
        utils::ascii_art("cockbalt");
        std::cout << "\nWelcome to ad.m CSGO Loader.\njust a simple LoadLibrary injector for my CSGO V2 Project.\n\n";

        // if the dll path dosent exist
        if (!utils::FileExists(csgo.dllPath)) {
            // make a choice to either enter a new path or shut down the program.
            if (utils::choice("Could not find the file '%s'!\nWould you like to enter a new dll file path or exit? y/n: ", csgo.dllPath)) {
                std::string buf;
                std::cout << dye::yellow("\nEnter a new dll path file: ");
                std::cin >> buf;
                csgo.dllPath = buf.c_str();
            }
            else {
                std::cout << dye::red("\nExiting in 3s...");
                Sleep(3000);
                ExitProcess(0);
            }
        }

        if (!csgo.isActive())
        {
            // for it to only execute ONCE.
            if (insecure == -1) {
                // Choose to start CSGO with -insecure or without.
                insecure = utils::choice("(recommended) Start %s with '-insecure'? y/n: ", csgo.Name);
            }

            std::cout << dye::aqua(std::format("\nLaunching {}{}...\n", csgo.Name, insecure ? " With -inseucre" : ""));
            {
                // this will always launch CSGO with the ID 730, to get your game's ID check steam.inf in your game folder and grab it from there <3.
                std::string tmp = insecure ? " -insecure" : "";
                std::string cmd = "\"C:\\Program Files (x86)\\Steam\\steam.exe\" -applaunch 730" + tmp;
                system(cmd.c_str());
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
        Sleep(7500); // wait for the game to fully open. (assuming it wasnt already opened)
        if (!Process::inject(csgo))
        {
            std::cout << dye::red("\ninjection Failed! Exiting in 3s...");    
            Sleep(3000);
            ExitProcess(0);
        }

        std::cout << dye::purple("injection Completed!\n") << dye::yellow("info:\n");
        printf("Dll path: %s\nProc Name: %s\nPID: %d", csgo.dllPath, csgo.Name, csgo.pid);

        system("pause>nul");

        // for reinjection
        Process::Terminate(csgo);
        Sleep(1000);
    }
}