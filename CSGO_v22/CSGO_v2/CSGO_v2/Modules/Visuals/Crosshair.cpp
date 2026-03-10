#include "Crosshair.h"
#include "dx9/Drawing/drawing.h"
#include "SDK/Globals/Globals.h"
#include "SDK/Entity/localplayer.h"
#include "lib/Hooks/hook.h"
#include "imgui.h"
#include "Animation.h"
#include <cmath>

// Crosshair styles
enum eCrosshairStyle : int {
	Style_Cross = 0,
	Style_Circle = 1,
	Style_Dot = 2,
	Style_CrossDot = 3
};

// Smooth interpolation helper — lerps current toward target each frame
static float SmoothLerp(float current, float target, float speed)
{
	float delta = target - current;
	if (std::abs(delta) < 0.01f) return target;
	float t = speed * ImGui::GetIO().DeltaTime;
	if (t > 1.f) t = 1.f;
	return current + delta * t;
}

// Persistent animated state
static struct {
	Animation fadeIn{0.3f, EaseInOutSine, EaseInOutSine};
	Animation recoilFade{0.15f, EaseInSine, EaseOutSine};

	// Smoothly interpolated values
	float size = 5.f;
	float gap = 2.f;
	float thickness = 1.f;

	// Smoothly interpolated recoil position
	float recoilX = 0.f;
	float recoilY = 0.f;
} anim;

// Draw a single crosshair at the given screen position
static void DrawCrosshairAt(float cx, float cy, float size, float gap, float thick,
	bool outline, int style, ImColor color, ImColor outlineColor)
{
	auto* dl = ImGui::GetBackgroundDrawList();

	bool drawCross = (style == Style_Cross || style == Style_CrossDot);
	bool drawDot = (style == Style_Dot || style == Style_CrossDot);
	bool drawCircle = (style == Style_Circle);

	if (drawCross) {
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
		if (outline)
			dl->AddCircleFilled(ImVec2(cx, cy), thick + 1.f, outlineColor);
		dl->AddCircleFilled(ImVec2(cx, cy), thick, color);
	}
}

void Crosshair::Render()
{
	auto& ch = cfg.visuals.crosshair;

	// Update fade animation (runs even when disabled so it can fade out)
	anim.fadeIn.Update();
	anim.fadeIn.Switch(ch.Enabled);

	float opacity = anim.fadeIn.getValue();
	if (opacity < 0.01f)
		return;

	if (!globals::g_interfaces.Engine->IsInGame())
		return;

	gEntity* lp = LocalPlayer.Get();
	if (!lp)
		return;

	bool alive = *(int*)((uintptr_t)lp + offsets::deadFlag) == 0;
	if (!alive)
		return;

	// Sniper crosshair check
	bool scoped = lp->isScoped();
	if (scoped && !ch.SniperCrosshair)
		return;

	// Smoothly interpolate size/gap/thickness toward config values
	float lerpSpeed = 12.f;
	anim.size = SmoothLerp(anim.size, ch.Size, lerpSpeed);
	anim.gap = SmoothLerp(anim.gap, ch.Gap, lerpSpeed);
	anim.thickness = SmoothLerp(anim.thickness, ch.Thickness, lerpSpeed);

	auto dispSize = ImGui::GetIO().DisplaySize;
	float centerX = dispSize.x / 2.f;
	float centerY = dispSize.y / 2.f;

	// Apply fade to colors
	ImColor mainColor(ch.Color.r, ch.Color.g, ch.Color.b, ch.Color.a * opacity);
	ImColor outlineCol(0.f, 0.f, 0.f, ch.Color.a * 0.8f * opacity);

	DrawCrosshairAt(centerX, centerY, anim.size, anim.gap, anim.thickness,
		ch.Outline, ch.Style, mainColor, outlineCol);

	// Recoil crosshair
	if (ch.RecoilCrosshair) {
		math::Vector punch = lp->getAimPunch();

		float pixelsPerDeg = dispSize.x / (globals::camFOV * 2.f);
		float targetX = -punch.y * 2.f * pixelsPerDeg;
		float targetY = punch.x * 2.f * pixelsPerDeg;

		// Smooth the recoil position for fluid movement
		float recoilLerp = 18.f;
		anim.recoilX = SmoothLerp(anim.recoilX, targetX, recoilLerp);
		anim.recoilY = SmoothLerp(anim.recoilY, targetY, recoilLerp);

		// Fade recoil crosshair based on whether there's actual recoil
		bool hasRecoil = std::abs(punch.x) > 0.01f || std::abs(punch.y) > 0.01f;
		anim.recoilFade.Update();
		anim.recoilFade.Switch(hasRecoil);
		float recoilOpacity = anim.recoilFade.getValue() * opacity;

		if (recoilOpacity > 0.01f) {
			ImColor recoilColor(ch.RecoilColor.r, ch.RecoilColor.g, ch.RecoilColor.b, ch.RecoilColor.a * recoilOpacity);
			ImColor recoilOutline(0.f, 0.f, 0.f, ch.RecoilColor.a * 0.8f * recoilOpacity);

			DrawCrosshairAt(centerX + anim.recoilX, centerY + anim.recoilY,
				anim.size, anim.gap, anim.thickness,
				ch.Outline, ch.Style, recoilColor, recoilOutline);
		}
	} else {
		// Reset recoil position when disabled so it doesn't jump when re-enabled
		anim.recoilX = 0.f;
		anim.recoilY = 0.f;
	}
}
