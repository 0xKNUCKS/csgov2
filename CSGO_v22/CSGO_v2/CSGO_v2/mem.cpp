#include "mem.h"

void mem::Patch(BYTE* dst, BYTE* src, unsigned int size)
{
	DWORD oProc;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oProc);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oProc, &oProc);
}

bool mem::Detour(char* src, char* dst, int len)
{
	if (len < 5) return 0;
	//setup
	DWORD oProc;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &oProc);
	memset(src, 0x90, len);

	//hook
	uintptr_t relAddy = (uintptr_t)(dst - src - 5);
	*src = (char)0xE9; // jmp
	*(uintptr_t*)(src + 1) = (uintptr_t)relAddy;

	//clean up
	VirtualProtect(src, len, oProc, &oProc);

	return true;
}

char* mem::TrampHook(char* src, char* dst, unsigned int len)
{
	if (len < 5) return 0;

	// allocate gateway
	char* gateway = (char*)VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// copy function to gateway
	if (gateway)
		memcpy(gateway, src, len);
	else
	{
		MessageBox(0, "Couldnt Allocate Memory for \"mem::TrampHook\"", "Fatal Error", MB_ICONERROR);
		exit(0);
	}

	// get gate way jump addr
	uintptr_t jumpAddy = (uintptr_t)(src - gateway - 5);

	// set jump (hook)
	*(gateway + len) = (char)0xE9;
	*(uintptr_t*)(gateway + len + 1) = jumpAddy;

	if (mem::Detour(src, dst, len))
		return gateway;
	else
		return nullptr;
}
