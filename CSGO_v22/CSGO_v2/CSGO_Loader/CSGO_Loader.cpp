#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <format>

#include "Process.h"
#include "color.hpp"
#include "utils.h"

// Process and DLL info.
#define PROC_NAME "csgo.exe"
#define WIND_NAME "Counter-Strike: Global Offensive - Direct3D 9"
#define CLSS_NAME "Valve001"
#define DLL_PATH  "CSGO_v2.dll"

// CSGO process class
proc_t csgo(PROC_NAME, "", WIND_NAME, CLSS_NAME);
int insecure = -1;

// updates every 100ms
void update()
{
    while (true)
    {
        csgo.hwnd = FindWindow(csgo.className.c_str(), csgo.windowName.c_str());

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

    // Resolve DLL path relative to the exe's directory
    csgo.dllPath = utils::ResolvePathRelativeToExe(DLL_PATH);

    CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(update), nullptr, 0, nullptr);
    Sleep(200);

    while (true)
    {
        system("cls");
        utils::ascii_art("cockbalt");
        std::cout << "\nWelcome to ad.m CSGO Loader.\njust a simple LoadLibrary injector for my CSGO V2 Project.\n\n";

        // if the dll path doesn't exist
        if (!utils::FileExists(csgo.dllPath)) {
            // make a choice to either enter a new path or shut down the program.
            if (utils::choice(std::format("Could not find the file '{}'!\nWould you like to enter a new dll file path or exit? y/n: ", csgo.dllPath))) {
                std::cout << dye::yellow("\nEnter a new dll path file: ");
                std::cin >> csgo.dllPath;
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
                insecure = utils::choice(std::format("(recommended) Start {} with '-insecure'? y/n: ", csgo.Name));
            }

            std::cout << dye::aqua(std::format("\nLaunching {}{}...\n", csgo.Name, insecure ? " With -insecure" : ""));
            {
                // this will always launch CSGO with the ID 730, to get your game's ID check steam.inf in your game folder and grab it from there <3.
                std::string tmp = insecure ? " -insecure" : "";
                std::string cmd = "\"C:\\Program Files (x86)\\Steam\\steam.exe\" -applaunch 730" + tmp;
                system(cmd.c_str());
            }

            do { Sleep(50); } while (!csgo.isActive());
        }

        {
            auto err = Process::GetProcID(csgo);
            if (!err) {
                err.print();
                std::cout << dye::red("Exiting in 3s...");
                Sleep(3000);
                ExitProcess(0);
            }
        }

        std::cout << dye::green("injecting...\n");
        Sleep(7500); // wait for the game to fully open. (assuming it wasnt already opened)

        {
            auto err = Process::inject(csgo);
            if (!err) {
                err.print();
                std::cout << dye::red("\nExiting in 3s...");
                Sleep(3000);
                ExitProcess(0);
            }
        }

        std::cout << dye::purple("injection Completed!\n") << dye::yellow("info:\n");
        std::cout << std::format("Dll path: {}\nProc Name: {}\nPID: {}", csgo.dllPath, csgo.Name, csgo.pid);

        system("pause>nul");

        // for reinjection
        {
            auto err = Process::Terminate(csgo);
            if (!err) {
                err.print();
                std::cout << dye::yellow("Warning: Could not terminate process cleanly. Continuing anyway...\n");
            }
        }
        Sleep(1000);
    }
}
