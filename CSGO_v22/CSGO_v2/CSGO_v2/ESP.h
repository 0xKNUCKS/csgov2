#pragma once
#include "drawing.h"

namespace ESP
{
	void Render();
	void DrawLine(BBox bbox);
	void DrawBoundingRect(BBox bbox, bool filled);
	void DrawBoundingBox(BBox bbox);
	void DrawHealthBar(BBox bbox, int health);
	void DrawName(BBox bbox, std::string name);
	void DrawSkeleton(gEntity* entity);

	inline float baseOpacity = 1.f;

	enum eBoxType : int
	{
		Outlined,
		Filled,
		Box3d,
		Corners
	};
}

