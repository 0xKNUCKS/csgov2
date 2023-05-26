#pragma once
#include "interface.h"
#include "netvars.h"
#include "config.h"
#include "entitylist.h"

namespace globals
{
	inline interfaces_t g_interfaces;
	inline NetVars_t g_NetVars;
	inline CEntityList EntList;
	inline CUserCmd* g_cmd;
	inline float camFOV = 90.0f;
	inline float aspectRatio = 2.f;

	namespace game
	{
		inline math::Matrix4x4 viewMatrix;
	}
};
