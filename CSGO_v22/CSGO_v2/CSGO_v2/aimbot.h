#pragma once
#include "utils.h"
#include "config.h"
#include "entity.h"

struct cTarget
{
    float fov = 0.0f;
    float distance = 0.0f;
    ent_t ent;
};

namespace aimbot
{
	inline cTarget Target;
	void Run(CUserCmd* cmd);
	void FindBestEnt(Config::Aimbot cfg, CUserCmd* cmd);
};

