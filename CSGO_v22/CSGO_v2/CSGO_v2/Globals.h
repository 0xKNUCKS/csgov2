#pragma once
#include "interface.h"
#include "netvars.h"
#include "entity.h"
#include "config.h"

namespace globals
{
	inline NetVars_t g_NetVars;
	inline interfaces_t g_interfaces;
	
	namespace game
	{
		inline math::Matrix4x4 viewMatrix;
	}
};
