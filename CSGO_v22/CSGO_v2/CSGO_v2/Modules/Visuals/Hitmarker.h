#pragma once

namespace hitmarker
{
	// Register/unregister game event listener (call during setup/teardown)
	void Init();
	void Shutdown();

	// Render hitmarker overlay (call from EndScene)
	void Render();
}
