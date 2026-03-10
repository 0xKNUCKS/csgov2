#include "Crosshair.h"
#include "dx9/Drawing/drawing.h"
#include "SDK/Globals/Globals.h"
#include "SDK/Entity/localplayer.h"
#include "lib/Hooks/hook.h"
#include "imgui.h"
#include <cmath>

// Crosshair styles
enum eCrosshairStyle : int {
	Style_Cross = 0,
	Style_Circle = 1,
	Style_Dot = 2,
	Style_CrossDot = 3
};

// Draw a single crosshair at the given screen position
static void DrawCrosshairAt(float cx, float cy, ImColor color, ImColor outlineColor)
{
	auto* dl = ImGui::GetBackgroundDrawList();
	float size = cfg.visuals.crosshair.Size;
	float gap = cfg.visuals.crosshair.Gap;
	float thick = cfg.visuals.crosshair.Thickness;
	bool outline = cfg.visuals.crosshair.Outline;
	int style = cfg.visuals.crosshair.Style;

	bool drawCross = (style == Style_Cross || style == Style_CrossDot);
	bool drawDot = (style == Style_Dot || style == Style_CrossDot);
	bool drawCircle = (style == Style_Circle);

	if (drawCross) {
		// Four arms with a gap in the center
		// Top
		if (outline) {
			dl->AddLine(ImVec2(cx, cy - gap - size), ImVec2(cx, cy - gap), outlineColor, thick + 2.f);
			dl->AddLine(ImVec2(cx, cy + gap), ImVec2(cx, cy + gap + size), outlineColor, thick + 2.f);
			dl->AddLine(ImVec2(cx - gap - size, cy), ImVec2(cx - gap, cy), outlineColor, thick + 2.f);
			dl->AddLine(ImVec2(cx + gap, cy), ImVec2(cx + gap + size, cy), outlineColor, thick + 2.f);
		}
		dl->AddLine(ImVec2(cx, cy - gap - size), ImVec2(cx, cy - gap), color, thick);
		dl->AddLine(ImVec2(cx, cy + gap), ImVec2(cx, cy + gap + size), color, thick);
		dl->AddLine(ImVec2(cx - gap - size, cy), ImVec2(cx - gap, cy), color, thick);
		dl->AddLine(ImVec2(cx + gap, cy), ImVec2(cx + gap + size, cy), color, thick);
	}

	if (drawDot) {
		float dotRadius = thick;
		if (outline)
			dl->AddCircleFilled(ImVec2(cx, cy), dotRadius + 1.f, outlineColor);
		dl->AddCircleFilled(ImVec2(cx, cy), dotRadius, color);
	}

	if (drawCircle) {
		if (outline)
			dl->AddCircle(ImVec2(cx, cy), size, outlineColor, 0, thick + 2.f);
		dl->AddCircle(ImVec2(cx, cy), size, color, 0, thick);
		// Center dot
		if (outline)
			dl->AddCircleFilled(ImVec2(cx, cy), thick + 1.f, outlineColor);
		dl->AddCircleFilled(ImVec2(cx, cy), thick, color);
	}
}

void Crosshair::Render()
{
	if (!cfg.visuals.crosshair.Enabled)
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	gEntity* lp = LocalPlayer.Get();
	if (!lp)
		return;

	bool alive = *(int*)((uintptr_t)lp + offsets::deadFlag) == 0;
	if (!alive)
		return;

	auto dispSize = ImGui::GetIO().DisplaySize;
	float centerX = dispSize.x / 2.f;
	float centerY = dispSize.y / 2.f;

	auto& ch = cfg.visuals.crosshair;
	ImColor mainColor(ch.Color.r, ch.Color.g, ch.Color.b, ch.Color.a);
	ImColor outline(0.f, 0.f, 0.f, ch.Color.a * 0.8f);

	// Sniper crosshair: only draw when scoped if enabled, or always draw if not scoped
	bool scoped = lp->isScoped();
	if (scoped && !ch.SniperCrosshair)
		return;

	// Draw main crosshair at screen center
	DrawCrosshairAt(centerX, centerY, mainColor, outline);

	// Recoil crosshair: draw a second crosshair offset by aim punch
	if (ch.RecoilCrosshair) {
		math::Vector punch = lp->getAimPunch();

		if (std::abs(punch.x) > 0.01f || std::abs(punch.y) > 0.01f) {
			// Convert punch angles to screen pixel offset
			// CS:GO applies punch * 2 to the view, and the screen maps camFOV degrees to screenWidth/2
			float pixelsPerDegH = dispSize.x / (globals::camFOV * 2.f);
			// Vertical FOV is derived from horizontal: vfov = hfov * (height/width)
			float pixelsPerDegV = pixelsPerDegH; // aspect ratio handled by uniform scaling

			float offsetX = -punch.y * 2.f * pixelsPerDegH;
			float offsetY = punch.x * 2.f * pixelsPerDegV;

			ImColor recoilColor(ch.RecoilColor.r, ch.RecoilColor.g, ch.RecoilColor.b, ch.RecoilColor.a);
			ImColor recoilOutline(0.f, 0.f, 0.f, ch.RecoilColor.a * 0.8f);

			DrawCrosshairAt(centerX + offsetX, centerY + offsetY, recoilColor, recoilOutline);
		}
	}
}
