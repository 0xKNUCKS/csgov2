#pragma once
#include "interface.h"
#include "netvars.h"
#include "config.h"
#include "entitylist.h"

namespace globals
{
	inline NetVars_t g_NetVars;
	inline interfaces_t g_interfaces;
	inline CEntityList EntList;
	
	namespace game
	{
		inline math::Matrix4x4 viewMatrix;
	}
};
