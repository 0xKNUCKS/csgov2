#pragma once
#include "SDK/Macros/VirtualMethod.h"

class KeyValues;

class IMaterial
{
public:
	VIRTUAL_METHOD(const char*, GetName, 0, (), (this))
	VIRTUAL_METHOD(const char*, GetTextureGroupName, 1, (), (this))
	VIRTUAL_METHOD(void, IncrementReferenceCount, 12, (), (this))
	VIRTUAL_METHOD(void, AlphaModulate, 27, (float alpha), (this, alpha))
	VIRTUAL_METHOD(void, ColorModulate, 28, (float r, float g, float b), (this, r, g, b))
	VIRTUAL_METHOD(void, SetMaterialVarFlag, 29, (int flag, bool on), (this, flag, on))
};

// Material var flags
enum MaterialVarFlags_t
{
	MATERIAL_VAR_IGNOREZ = (1 << 15),
	MATERIAL_VAR_WIREFRAME = (1 << 28),
};

class IMaterialSystem
{
public:
	VIRTUAL_METHOD(IMaterial*, CreateMaterial, 83, (const char* name, KeyValues* kv), (this, name, kv))
	VIRTUAL_METHOD(IMaterial*, FindMaterial, 84, (const char* name, const char* textureGroupName, bool complain = true, const char* complainPrefix = nullptr), (this, name, textureGroupName, complain, complainPrefix))
	VIRTUAL_METHOD(short, FirstMaterial, 86, (), (this))
	VIRTUAL_METHOD(short, NextMaterial, 87, (short handle), (this, handle))
	VIRTUAL_METHOD(short, InvalidMaterial, 88, (), (this))
	VIRTUAL_METHOD(IMaterial*, GetMaterial, 89, (short handle), (this, handle))
};
