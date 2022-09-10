#pragma once
#include "interface.h"
#include "netvars.h"
#include "config.h"

namespace globals
{
	inline NetVars_t g_NetVars;
	inline interfaces_t g_interfaces;
	//inline CEntityList EntList; // C++ is taking a shit so ToDo once i fix it...
	
	namespace game
	{
		inline math::Matrix4x4 viewMatrix;
	}
};
