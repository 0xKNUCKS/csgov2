#include "Process.h"
#include <format>

LoaderError Process::GetProcID(proc_t &proc)
{
	DWORD ProcID = 0;
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap == INVALID_HANDLE_VALUE)
		return LoaderError::FromLastError(LoaderError::Code::SnapshotFailed,
			"CreateToolhelp32Snapshot failed");

	if (Process32First(hSnap, &procEntry))
	{
		do
		{
			if (!_stricmp(procEntry.szExeFile, proc.Name.c_str()))
			{
				ProcID = procEntry.th32ProcessID;
				break;
			}
		} while (Process32Next(hSnap, &procEntry));
	}

	CloseHandle(hSnap); // always close, even if not found

	if (ProcID == 0)
		return LoaderError(LoaderError::Code::ProcessNotFound,
			std::format("Process '{}' not found in snapshot", proc.Name));

	proc.pid = ProcID;
	return LoaderError(); // OK
}

LoaderError Process::inject(const proc_t& proc)
{
	if (!utils::FileExists(proc.dllPath))
		return LoaderError(LoaderError::Code::FileNotFound,
			std::format("DLL file not found: '{}'", proc.dllPath));

	// Open a Handle to our Targeted process
	HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, proc.pid);

	if (!pHandle || pHandle == INVALID_HANDLE_VALUE)
		return LoaderError::FromLastError(LoaderError::Code::OpenProcessFailed,
			std::format("OpenProcess failed for PID {}", proc.pid));

	// Allocate some memory to store the dll's path
	size_t pathLen = proc.dllPath.size() + 1;
	void* Aloc = VirtualAllocEx(pHandle, 0, pathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!Aloc) {
		auto err = LoaderError::FromLastError(LoaderError::Code::AllocFailed,
			std::format("VirtualAllocEx failed ({} bytes in PID {})", pathLen, proc.pid));
		CloseHandle(pHandle);
		return err;
	}

	// write our dll path to the memory space allocated for it to be used by LoadLibraryA
	if (!WriteProcessMemory(pHandle, Aloc, proc.dllPath.c_str(), pathLen, 0)) {
		auto err = LoaderError::FromLastError(LoaderError::Code::WriteFailed,
			"WriteProcessMemory failed — could not write DLL path");
		VirtualFreeEx(pHandle, Aloc, 0, MEM_RELEASE);
		CloseHandle(pHandle);
		return err;
	}

	// Create a thread to execute LoadLibraryA with the parameter "Aloc" which is just the dll path, this will call LoadLibraryA(dllPath) which will result in loading our custom dll inside the process
	HANDLE hThread = CreateRemoteThread(pHandle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, Aloc, 0, 0);

	if (!hThread || hThread == INVALID_HANDLE_VALUE) {
		auto err = LoaderError::FromLastError(LoaderError::Code::ThreadCreationFailed,
			"CreateRemoteThread failed — could not start LoadLibraryA");
		VirtualFreeEx(pHandle, Aloc, 0, MEM_RELEASE);
		CloseHandle(pHandle);
		return err;
	}

	std::cout << "[Thread] Executing...\n";

	// this will wait for the thread to finish executing
	if (WaitForSingleObject(hThread, INFINITE) != WAIT_OBJECT_0)
	{
		auto err = LoaderError::FromLastError(LoaderError::Code::ThreadExecutionFailed,
			"WaitForSingleObject failed — remote thread did not complete");
		CloseHandle(hThread);
		VirtualFreeEx(pHandle, Aloc, 0, MEM_RELEASE);
		CloseHandle(pHandle);
		return err;
	}

	std::cout << "[Thread] Finished Executing!\n";

	LoaderError result; // OK by default

	// after the thread has finished executing we're gonna attempt to retrieve the exit code returned by the thread
	DWORD dwExitCode = 0;
	if (!GetExitCodeThread(hThread, &dwExitCode))
	{
		result = LoaderError::FromLastError(LoaderError::Code::ThreadExitCodeFailed,
			"GetExitCodeThread failed — cannot verify injection success");
	}
	else {
		std::cout << std::format("[Thread] Exited with code {:#x}!\n", dwExitCode);
		if (dwExitCode == 0) {
			result = LoaderError(LoaderError::Code::ThreadExecutionFailed,
				"LoadLibraryA returned 0 — DLL failed to load (check DLL dependencies/architecture)");
		}
	}

	// Cleanup: free the allocated memory in the target process, then close handles
	VirtualFreeEx(pHandle, Aloc, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(pHandle);

	return result;
}

LoaderError Process::Terminate(proc_t& proc)
{
	if (!proc.isActive())
		return LoaderError(LoaderError::Code::ProcessNotFound,
			"Process is not active — nothing to terminate");

	const auto procTermH = OpenProcess(PROCESS_TERMINATE, false, proc.pid);
	if (!procTermH || procTermH == INVALID_HANDLE_VALUE)
		return LoaderError::FromLastError(LoaderError::Code::OpenProcessFailed,
			std::format("OpenProcess(TERMINATE) failed for PID {}", proc.pid));

	if (!TerminateProcess(procTermH, 0)) {
		auto err = LoaderError::FromLastError(LoaderError::Code::TerminateFailed,
			std::format("TerminateProcess failed for PID {}", proc.pid));
		CloseHandle(procTermH);
		return err;
	}

	CloseHandle(procTermH);
	proc.pid = 0;
	proc.hwnd = nullptr;
	return LoaderError(); // OK
}
