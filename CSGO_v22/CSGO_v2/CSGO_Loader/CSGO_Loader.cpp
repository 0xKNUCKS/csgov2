#include <iostream>
#include "Process.h"
#include "color.hpp"

int main()
{
    SetConsoleTitleA("cockbalt.solutions");

    proc_t csgo;
    csgo.dllPath = "CSGO_v2.dll"; // C:\Users\96565\source\repos\CSGO_v2\Debug\CSGO_v2.dll
    csgo.Name = "csgo.exe";

    while (true)
    {
        system("cls");
        std::cout << "Welcome to ad.m CSGO Loader.\njust a simple loadlibrary injector for CSGO_v2.\n\n";
        std::cout << dye::yellow("Press any key to start!\n");
        system("pause>nul");

        if (!Process::GetProcID(csgo))
        {
            std::cout << dye::aqua("\nWaiting for CSGO...\n");

            while (!Process::GetProcID(csgo))
            {
                Process::GetProcID(csgo);
            }
        }

        std::cout << dye::green("injecting...\n");
        if (!Process::inject(csgo))
        {
            std::cout << dye::red("injection Failed! Exiting in 3s...");
            Sleep(3000);
            exit(0);
        }
        
        std::cout << dye::purple("injection Completed!\n") << dye::yellow("info:\n");
        std::cout << ("dll path: " + (const char)csgo.dllPath) << ("\nProc Name: " + (const char)csgo.Name) << ("\npid: " + csgo.pid);
        
        system("pause>nul");
    }
}