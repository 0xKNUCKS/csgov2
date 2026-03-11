#pragma once
#include "SDK/Macros/VirtualMethod.h"
#include "SDK/Models/Model.h"
#include <cstring>

#include "SDK/Classes/MaterialSystem.h"

struct ModelRenderInfo_t
{
	math::Vector origin;
	math::Vector angles;
	char pad[4];
	void* pRenderable;
	const model_t* pModel;
	const math::Matrix3x4* pModelToWorld;
	const math::Matrix3x4* pLightingOffset;
	const math::Vector* pLightingOrigin;
	int flags;
	int entity_index;
	int skin;
	int body;
	int hitboxset;
	unsigned short instance;
};

struct DrawModelState_t;

class IVModelRender
{
public:
	VIRTUAL_METHOD(void, ForcedMaterialOverride, 1, (IMaterial* mat, int overrideType = 0, int overrides = 0), (this, mat, overrideType, overrides))
};

enum class OverrideType : int
{
	Normal = 0,
	BuildShadows,
	DepthWrite,
	CustomMaterial,
	SsaoDepthWrite
};

class IStudioRender
{
public:
	VIRTUAL_METHOD(void, ForcedMaterialOverride, 33, (IMaterial* mat, int overrideType = 0), (this, mat, overrideType))

	// Check if engine already has a forced override active (glow pass, depth write, etc.)
	// Matches CStudioRenderContext::IsForcedMaterialOverride logic
	bool IsForcedMaterialOverride() noexcept
	{
		// Offsets from Osiris: Win32 pad=592 to materialOverride, then pad 12 to overrideType
		auto* base = reinterpret_cast<char*>(this);
		auto* matOverride = *reinterpret_cast<IMaterial**>(base + 592);
		auto ovType = *reinterpret_cast<OverrideType*>(base + 592 + sizeof(void*) + 12);

		if (!matOverride)
			return ovType == OverrideType::DepthWrite || ovType == OverrideType::SsaoDepthWrite;

		// Check if it's a glow material
		const char* name = matOverride->GetName();
		if (name && strncmp(name, "dev/glow", 8) == 0)
			return true;

		return false;
	}
};
