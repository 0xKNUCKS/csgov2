#pragma once
#include "drawing.h"
//#include "entitylist.h"

namespace ESP
{
	void Render();
	void DrawLine(const math::Vector& screenPos);
	void DrawBoundingBox(gEntity* ent, const math::Vector& screenPos);
}

