#pragma once
#include "utils.h"
#include "config.h"
#include "entity.h"

#define norm(x) (float)(int)x

struct cTarget
{
    float fov = 0.0f;
    float distance = 0.0f;
    gEntity* ent;
};

namespace aimbot
{
	inline cTarget Target;
	void Run(CUserCmd* cmd);
};

