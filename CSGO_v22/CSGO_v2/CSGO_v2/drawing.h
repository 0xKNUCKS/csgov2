#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include "GUI.h"
#include "config.h"
#include "math.h"

enum eBoxType : UINT16
{
	Filled,
	Outlined,
	Box3d,
	Corners
};

namespace Render
{
	void Line(float fromX, float fromY, float toX, float toY, float thickness = 1.0F, ImColor color = ImColor(255, 255, 255));

	void OutLinedRect(float x, float y, int w, int h, float Thickness = 1.0F, ImColor color = ImColor(255, 255, 255));
	void FilledRect				(float x, float y, int w, int h, ImColor color = ImColor(255, 255, 255));

	void CenteredOutlinedRect	(float x, float y, float w, float h, ImColor color = ImColor(255, 255, 255));
	void CenteredFilledRect		(float x, float y, float w, float h, ImColor color = ImColor(255, 255, 255));

	void OutLinedCircle			(float x, float y, float rad, ImColor color = ImColor(255,255,255));
	void FilledCircle			(float x, float y, float rad, ImColor color = ImColor(255, 255, 255));

	void OutLinedText(const  char* text, float x, float y, ImDrawList* drawList, ImColor color = ImColor(255, 255, 255));

	namespace ESP
	{
		void DrawBox(math::Vector top, math::Vector bot, ImColor color, eBoxType Type);
	}

	bool WolrdToScreen(math::Vector& in, math::Vector& out);
};

