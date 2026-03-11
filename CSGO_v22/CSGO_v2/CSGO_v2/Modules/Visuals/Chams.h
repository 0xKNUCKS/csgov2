#pragma once
#include "SDK/Classes/ModelRender.h"

namespace chams
{
	bool Init();
	void Shutdown();

	// Result of OnDrawModel
	enum class Result {
		None,           // No chams — proceed normally
		VisibleOnly,    // Material override set for visible pass only
		ThroughWalls    // Material override set for invisible pass; caller must draw,
		                // then call SetupVisiblePass() and draw again
	};

	// Sets up material override. Returns how to draw.
	Result OnDrawModel(const ModelRenderInfo_t& info);

	// After invisible pass, call this to set up the visible pass material.
	void SetupVisiblePass();

	// Clear material override after drawing.
	void ClearOverride();
}
