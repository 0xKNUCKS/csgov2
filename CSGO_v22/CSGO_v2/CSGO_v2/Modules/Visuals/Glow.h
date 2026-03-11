#pragma once
#include "SDK/Entity/entity.h"
#include "lib/Math/GameMath.h"

struct GlowObjectDefinition_t {
	// Runtime layout (verified via debug dump — nextFreeSlot is FIRST, not last)
	int nextFreeSlot;                       // 0x00
	gEntity* pEntity;                       // 0x04
	float r, g, b;                          // 0x08 (Vector - 3 floats)
	float a;                                // 0x14 (glowAlpha)
	bool glowAlphaCappedByRenderAlpha;      // 0x18
	char pad1[3];                           // 0x19 (alignment)
	float glowAlphaFuncOfMaxVelocity;       // 0x1C
	float glowAlphaMax;                     // 0x20
	float glowPulseOverdrive;               // 0x24
	bool renderWhenOccluded;                // 0x28
	bool renderWhenUnoccluded;              // 0x29
	bool fullBloomRender;                   // 0x2A
	char pad2[1];                           // 0x2B (alignment)
	int fullBloomStencilTestValue;          // 0x2C
	int glowStyle;                          // 0x30 (m_nRenderStyle)
	int splitScreenSlot;                    // 0x34

	static constexpr int END_OF_FREE_LIST = -1;
	static constexpr int ENTRY_IN_USE = -2;

	bool isUnused() const { return nextFreeSlot != ENTRY_IN_USE; }
};

struct GlowObjectManager_t {
	math::UtlVector<GlowObjectDefinition_t> glowObjects;
};

namespace glow
{
	inline GlowObjectManager_t* glowManager = nullptr;

	// Find GlowObjectManager via pattern scan (call once during init)
	bool Init();

	// Apply glow to entities (call in FrameStageNotify or CreateMove)
	void Run();
}
