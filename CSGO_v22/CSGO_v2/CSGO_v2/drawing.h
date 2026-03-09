#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#include "GUI.h"
#include "config.h"
#include "GameMath.h"

namespace Render
{
	void Line(float fromX, float fromY, float toX, float toY, float thickness = 1.0F, ImColor color = ImColor(255, 255, 255));

	void OutLinedRect(float x, float y, float w, float h, float Thickness = 1.0F, ImColor color = ImColor(255, 255, 255));
	void FilledRect				(float x, float y, float w, float h, ImColor color = ImColor(255, 255, 255));

	void CenteredOutlinedRect	(float x, float y, float w, float h, ImColor color = ImColor(255, 255, 255));
	void CenteredFilledRect		(float x, float y, float w, float h, ImColor color = ImColor(255, 255, 255));

	void OutLinedCircle			(float x, float y, float rad, ImColor color = ImColor(255,255,255));
	void FilledCircle			(float x, float y, float rad, ImColor color = ImColor(255, 255, 255));

	void OutLinedText(const  char* text, float x, float y, ImDrawList* drawList, ImColor color = ImColor(255, 255, 255));
};

