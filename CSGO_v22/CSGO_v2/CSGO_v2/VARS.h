#pragma once
#include "entity.h"
#include "drawing.h"
#include "math.h"
//#include "hook.h"

//class ent_t;

namespace SDK
{
	namespace Variables
	{
		static uintptr_t dwEngine;
		static uintptr_t dwClient;
		//static 
		static float viewMatrix[16];
		inline interfaces_t interfff;

		void UpdateMatrix(interfaces_t interfff);
	};

	namespace GAME
	{
		bool WorldToScreen(math::Vector pos, math::Vector& Screen, interfaces_t interfff);
	}
}
