#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <format>

#include "Process.h"
#include "color.hpp"
#include "utils.h"
#include "MD5.h"

// Process and DLL info.
#define PROC_NAME "csgo.exe"
#define WIND_NAME "Counter-Strike: Global Offensive - Direct3D 9"
#define CLSS_NAME "Valve001"
#define DLL_PATH  "CSGO_v2.dll"

// CSGO process class
proc_t csgo(PROC_NAME, "", WIND_NAME, CLSS_NAME);

// Hash a file's contents with MD5
static std::string HashFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return "";
    std::string contents((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
    return md5(contents);
}

// Track last injected DLL hash
static std::string lastInjectedHash;

// Main function
int main(void)
{
    SetConsoleTitleA("cockbalt.solutions - (Build: " __DATE__ ")");

    // Resolve DLL path relative to the exe's directory
    csgo.dllPath = utils::ResolvePathRelativeToExe(DLL_PATH);

    while (true)
    {
        system("cls");
        utils::ascii_art("cockbalt");
        std::cout << "\nWelcome to ad.m CSGO Loader.\njust a simple LoadLibrary injector for my CSGO V2 Project.\n\n";

        // if the dll path doesn't exist
        if (!utils::FileExists(csgo.dllPath)) {
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

        // Check if DLL has changed since last injection
        std::string currentHash = HashFile(csgo.dllPath);
        if (!currentHash.empty() && !lastInjectedHash.empty()) {
            if (currentHash == lastInjectedHash)
                std::cout << dye::yellow("[DLL] Same as last injection (no changes detected)\n");
            else
                std::cout << dye::green("[DLL] New build detected!\n");
        }

        // Reset process state for a fresh attempt
        csgo.pid = 0;
        csgo.hwnd = nullptr;

        std::cout << dye::aqua("Press ENTER when CS:GO is open and you're ready to inject...");
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');

        // Find the process
        csgo.hwnd = FindWindow(csgo.className.c_str(), csgo.windowName.c_str());
        {
            auto err = Process::GetProcID(csgo);
            if (!err) {
                err.print();
                std::cout << dye::red("\nCS:GO doesn't seem to be running. Press ENTER to try again...");
                std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                continue;
            }
        }

        std::cout << dye::green("\nCS:GO found! Injecting...\n");
        Sleep(1000);

        {
            auto err = Process::inject(csgo);
            if (!err) {
                err.print();
                std::cout << dye::red("\nInjection failed. Press ENTER to try again...");
                std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                continue;
            }
        }

        // Update the last injected hash
        lastInjectedHash = currentHash;

        std::cout << dye::purple("\nInjection Completed!\n") << dye::yellow("Info:\n");
        std::cout << std::format("  Dll path:  {}\n  Proc Name: {}\n  PID:       {}\n  DLL MD5:   {}\n", csgo.dllPath, csgo.Name, csgo.pid, lastInjectedHash);

        std::cout << dye::aqua("\nPress ENTER to inject again (re-open CS:GO first)...");
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    }
}
