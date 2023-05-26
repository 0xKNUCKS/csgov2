#pragma once
#include "utils.h"
#include "config.h"
#include "entity.h"

#define norm(x) (float)(int)x

struct cTarget
{
    float fov = 0.0f;
    float distance = 0.0f;
    ent_t ent;
};

namespace aimbot
{
	inline cTarget Target;
	inline std::vector<std::pair<float, ent_t>> TargetsArr;
	void Run(CUserCmd* cmd);
	std::vector < std::pair <float, ent_t >> GetTargetsArr(Config::Aimbot cfg);
};

