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

	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, proc.pid);

	if (!pHandle || pHandle == INVALID_HANDLE_VALUE)
		return 0;

	void* Aloc = VirtualAllocEx(pHandle, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (Aloc)
		if (!WriteProcessMemory(pHandle, Aloc, proc.dllPath, strlen(proc.dllPath) + 1, 0))
			return 0;

	HANDLE hThread = CreateRemoteThread(pHandle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, Aloc, 0, 0);

	if (hThread) { CloseHandle(hThread); }
	if (pHandle) { CloseHandle(pHandle); }

	return 1;
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