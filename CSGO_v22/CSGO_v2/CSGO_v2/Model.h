#pragma once
#include "VirtualMethod.h"
#include "math.h"
#include "Pad.h"

// Macros
// source: https://github.com/perilouswithadollarsign/cstrike15_src/blob/master/public/studio.h
#define MAXSTUDIOBONES		256		// total bones actually used
#define BONE_USED_BY_ANYTHING		0x000FFF00
#define BONE_USED_BY_HITBOX			0x00000100	// bone (or child) is used by a hit box
#define BONE_USED_BY_ATTACHMENT		0x00000200	// bone (or child) is used by an attachment point


enum Hitbox:int
{
	Hitbox_Head,
	Hitbox_Neck,
	Hitbox_Pelvis,
	Hitbox_Belly,
	Hitbox_Thorax,
	Hitbox_LowerChest,
	Hitbox_UpperChest,
	Hitbox_RightThigh,
	Hitbox_LeftThigh,
	Hitbox_RightCalf,
	Hitbox_LeftCalf,
	Hitbox_RightFoot,
	Hitbox_LeftFoot,
	Hitbox_RightHand,
	Hitbox_LeftHand,
	Hitbox_RightUpperArm,
	Hitbox_RightForearm,
	Hitbox_LeftUpperArm,
	Hitbox_LeftForearm,
	Hitbox_Max
};

struct model_t {
	void* handle;
	char name[260];
	int	loadFlags;
	int	serverCount;
	int	type;
	int	flags;
	math::Vector mins, maxs;
};

// bones
struct mstudiobone_t
{
	int					sznameindex;
	inline const char* const pszName(void) const { return ((const char*)this) + sznameindex; }
	int		 			parent;		// parent bone
	int					bonecontroller[6];	// bone controller index, -1 == none
	PAD(152)
	int					flags;
};

struct studiohdr_t
{
	int					id;
	int					version;

	int				checksum;		// this has to be the same in the phy and vtx files to load!

	char				name[64];

	int					length;

	math::Vector				eyeposition;	// ideal eye position

	math::Vector				illumposition;	// illumination center

	math::Vector				hull_min;		// ideal movement hull size
	math::Vector				hull_max;

	math::Vector				view_bbmin;		// clipping bounding box
	math::Vector				view_bbmax;

	int					flags;

	int					numbones;			// bones
	int					boneindex;
	inline const mstudiobone_t* pBone(int i) const { return (i >= 0 && i < numbones) ? (mstudiobone_t*)(((uintptr_t*)this) + boneindex) + i : nullptr; };
};

struct IVModelInfo
{
	// virtual studiohdr_t				*GetStudiomodel( const model_t *mod ) = 0;
	VIRTUAL_METHOD(studiohdr_t*, GetStudioModel, 32, (const model_t* mod), (this, mod))
};