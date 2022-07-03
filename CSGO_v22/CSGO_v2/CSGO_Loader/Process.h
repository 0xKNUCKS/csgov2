#pragma once
#include <Windows.h>
#include <TlHelp32.h>

struct proc_t
{
	const char* Name;
	const char* dllPath;
	DWORD pid;
};

namespace Process
{
	BOOL GetProcID(proc_t& proc);

	BOOL inject(proc_t proc);
}

