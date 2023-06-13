#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include "utils.h"
#include "color.hpp"

struct proc_t;

namespace Process
{
	bool GetProcID(proc_t& proc);
	bool inject(proc_t proc);
	bool Terminate(proc_t proc);
}

struct proc_t
{
	proc_t(const char* name, const char* dllpath) { this->Name = name; this->dllPath = dllpath; };
	proc_t(const char* name, const char* dllpath, const char* windowname, const char* classname) { this->Name = name; this->dllPath = dllpath; this->windowName = windowname; this->className = classname; }

	const char* Name;
	const char* dllPath;
	
	const char* windowName;
	const char* className;

	DWORD pid;
	HWND hwnd;

	bool isActive()
	{ return hwnd || Process::GetProcID(*this); }
};

