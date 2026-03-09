#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include "utils.h"
#include "LoaderError.h"

struct proc_t;

namespace Process
{
	LoaderError GetProcID(proc_t& proc);
	LoaderError inject(const proc_t& proc);
	LoaderError Terminate(proc_t& proc);
}

struct proc_t
{
	proc_t(std::string name, std::string dllpath)
		: Name(std::move(name)), dllPath(std::move(dllpath)) {}

	proc_t(std::string name, std::string dllpath, std::string windowname, std::string classname)
		: Name(std::move(name)), dllPath(std::move(dllpath)),
		  windowName(std::move(windowname)), className(std::move(classname)) {}

	std::string Name;
	std::string dllPath;
	std::string windowName;
	std::string className;

	DWORD pid = 0;
	HWND hwnd = nullptr;

	bool isActive()
	{ return hwnd || Process::GetProcID(*this).ok(); }
};

