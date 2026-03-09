#pragma once
#include "SDK/interface/interface.h"
#include "SDK/netvars/netvars.h"
#include "lib/Configs/config.h"

namespace globals
{
	inline interfaces_t g_interfaces;
	inline NetVars_t g_NetVars;
	inline CUserCmd* g_cmd;
	inline float camFOV = 90.0f;
	inline float aspectRatio = 2.f;

	namespace game
	{
		inline math::Matrix4x4 viewMatrix;
	}
};
