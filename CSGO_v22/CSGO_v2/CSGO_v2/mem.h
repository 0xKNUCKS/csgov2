#pragma once
#include <Windows.h>

namespace mem
{
	void Patch(BYTE* dst, BYTE* src, unsigned int size);

	bool Detour(char* src, char* dst, int len);

	char* TrampHook(char* src, char* dst, unsigned int len);
};

