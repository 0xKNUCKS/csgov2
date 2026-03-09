#pragma once

#include <cstdint>

class SetupCRC
{
	uint32_t ulCrc = 0xFFFFFFFF;

public:
	constexpr operator uint32_t() noexcept
	{
		return ulCrc ^ 0xFFFFFFFF;
	}

	void ProcessBuffer(const void* buffer, unsigned size) noexcept;
};