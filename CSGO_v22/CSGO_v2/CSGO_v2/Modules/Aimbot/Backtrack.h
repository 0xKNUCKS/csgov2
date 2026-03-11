#pragma once
#include "lib/Math/GameMath.h"
#include "SDK/Entity/entity.h"
#include <deque>
#include <array>

// Maximum ticks we can backtrack (server allows ~200ms, ~12-13 ticks at 64 tick)
constexpr int BACKTRACK_MAX_TICKS = 12;
constexpr int MAX_PLAYERS = 65;

struct BacktrackRecord
{
	float simTime = 0.f;
	math::Vector headPos;
	math::Vector origin;
	math::Matrix3x4 boneMatrix[128]; // full bone snapshot
	bool valid = false;
};

namespace backtrack
{
	inline std::array<std::deque<BacktrackRecord>, MAX_PLAYERS> records;

	// Call every tick in CreateMove to store player states
	void Update();

	// Standalone backtrack: when shooting without aimbot, find best past-tick near crosshair
	void Run(CUserCmd* cmd);

	// Find the best backtrack record for the current aimbot target
	// Returns true if a valid backtrack tick was found, fills outRecord
	bool GetBestRecord(gEntity* localPlayer, const math::Vector& localEyePos,
		const math::Vector& viewAngles, float maxFov, BacktrackRecord& outRecord, int& outPlayerIdx);

	// Check if a record's tick is still valid (within server latency window)
	bool IsTickValid(float simTime);

	// Apply backtrack to cmd (set tick_count)
	void Apply(CUserCmd* cmd, float simTime);

	// Clear all stored records (call on map change, etc.)
	void Clear();

	// Get the number of stored records for debug display
	int GetTotalRecords();

	// Render backtrack tick dots (SEH-safe, called from EndScene)
	void RenderTicks();
}
