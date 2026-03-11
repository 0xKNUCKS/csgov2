#pragma once
#include "lib/utils/utils.h"
#include "lib/Configs/config.h"
#include "SDK/Entity/entity.h"

struct cTarget
{
    float fov = 0.0f;
    float distance = 0.0f;
    gEntity* ent = nullptr;
    math::Vector bonePos; // target bone position for visuals
};

namespace aimbot
{
	inline cTarget Target;
	void Run(CUserCmd* cmd);
	void RunInternal(CUserCmd* cmd);
};

