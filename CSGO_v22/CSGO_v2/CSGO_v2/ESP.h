#pragma once
#include "drawing.h"

namespace ESP
{
	void Render();
	void DrawLine(BBox bbox);
	void DrawBoundingRect(BBox bbox, bool filled);
	void DrawBoundingBox(BBox bbox);
	void DrawHealthBar(BBox bbox, int health);

	enum eBoxType : UINT16
	{
		Outlined,
		Filled,
		Box3d,
		Corners
	};
}

