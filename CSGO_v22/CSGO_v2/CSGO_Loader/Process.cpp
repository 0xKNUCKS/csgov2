#include "Process.h"

bool Process::GetProcID(proc_t &proc)
{
	DWORD ProcID = 0;
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap == INVALID_HANDLE_VALUE)
		return 0;

	if (Process32First(hSnap, &procEntry))
	{
		do
		{
			if (!_stricmp(procEntry.szExeFile, proc.Name))
			{
				ProcID = procEntry.th32ProcessID;
				break;
			}
		} while (Process32Next(hSnap, &procEntry));
	}

	if (ProcID <= 0)
		return 0;

	CloseHandle(hSnap);
	proc.pid = ProcID;
	return 1;
}

bool Process::inject(proc_t proc)
{
	/*ToDo: better error handling (throwing runtime errors)*/
	if (!utils::FileExists(proc.dllPath))
		return 0;

	// Lambda function for cleanup
	auto FreeH = [](HANDLE handle) {
		if (handle) {
			CloseHandle(handle);
		}
	};

	// Open a Handle to our Targeted process
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, proc.pid);

	if (!pHandle || pHandle == INVALID_HANDLE_VALUE)
		return 0;

	// Allocate some memory to store the dll's path
	void* Aloc = VirtualAllocEx(pHandle, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (Aloc) {
		// write our dll path to the memeory space allocated for it to be used by LoadLibraryA
		if (!WriteProcessMemory(pHandle, Aloc, proc.dllPath, strlen(proc.dllPath) + 1, 0)) {
			FreeH(pHandle);
			return 0;
		}
	}

	// Create a thread to execute LoadLibraryA with the parameter "Aloc" which is just the dll path, this wil call LoadLibraryA(dllPath) which will result in loading our custom dll inside the process
	HANDLE hThread = CreateRemoteThread(pHandle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, Aloc, 0, 0);
	
	if (!hThread || hThread == INVALID_HANDLE_VALUE)
		return 0;

	std::cout << ("[Thread] Executing...\n");

	// this will wait for the thread to finish executing
	if (WaitForSingleObject(hThread, INFINITE) != WAIT_OBJECT_0)
	{
		std::cout << (std::format("Thread Execution Failed! Error: {}\n", GetLastError()));
		FreeH(hThread);
		FreeH(pHandle);
		return 0;
	}

	std::cout << (std::format("[Thread] Finished Excecuting!\n"));

	// for recording the return's result to cleanup handles before returning
	bool result = 1;

	// after the thread has finished executing we're gonna attempt to retrieve the exit code returned by the thread
	DWORD dwExitCode = 0;
	if (!GetExitCodeThread(hThread, &dwExitCode) && !result)
	{
		std::cout << (std::format("Failed to retrieve Thread's return code! Error: {}\n", GetLastError()));
		result = 0;
	}
	else
		std::cout << (std::format("[Thread] Exited with code {}!\n", dwExitCode));

	// After were finished from our threads and handles were gonna clean them up
	FreeH(hThread);
	FreeH(pHandle);

	return result;
}

bool Process::Terminate(proc_t proc)
{
	if (proc.isActive())
	{
		const auto procTermH = OpenProcess(PROCESS_TERMINATE, false, proc.pid);
		if (procTermH != INVALID_HANDLE_VALUE) {
			TerminateProcess(procTermH, 0);
			CloseHandle(procTermH);
			return 1;
		}
		else return 0;
	}

	return 0;
}