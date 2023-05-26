#pragma once
#include "VirtualMethod.h"
#include "math.h"
#include "entity.h"

// Note: Shareddefs.h file in the sdk got most of the defs youll probably need

// trace_t Displacement flags 
#define DISPSURF_FLAG_SURFACE		(1<<0)
#define DISPSURF_FLAG_WALKABLE		(1<<1)
#define DISPSURF_FLAG_BUILDABLE		(1<<2)
#define DISPSURF_FLAG_SURFPROP1		(1<<3)
#define DISPSURF_FLAG_SURFPROP2		(1<<4)
#define DISPSURF_FLAG_SURFPROP3		(1<<5)
#define DISPSURF_FLAG_SURFPROP4		(1<<6)

// Hit Groups
enum eHitGroups_t
{
 HITGROUP_GENERIC,
 HITGROUP_HEAD,	
 HITGROUP_CHEST,
 HITGROUP_STOMACH,
 HITGROUP_LEFTARM,
 HITGROUP_RIGHTARM,
 HITGROUP_LEFTLEG,
 HITGROUP_RIGHTLEG,
 HITGROUP_GEAR = 10			// alerts NPC, but doesn't do damage or bleed (1/100th damage)
};

enum TraceType_t : int
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,				// NOTE: This does *not* test static props!!!
	TRACE_ENTITIES_ONLY,			// NOTE: This version will *not* test static props
	TRACE_EVERYTHING_FILTER_PROPS,	// NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
};

struct Ray_t
{
public:
	Ray_t(const math::Vector& src, const math::Vector& dest) {
		m_Start = src;
		m_Delta = dest - src;
		m_IsSwept = m_Delta.x || m_Delta.y || m_Delta.z;}

	math::Vector  m_Start;	// starting point, centered within the extents
	math::Vector  m_Delta;	// direction + length of the ray
	math::Vector  m_StartOffset;	// Add this to m_Start to get the actual ray start
	math::Vector  m_Extents;	// Describes an axis aligned box extruded along a ray
	const math::Matrix3x4* m_pWorldAxisTransform;
	bool	m_IsRay;	// are the extents zero?
	bool	m_IsSwept;	// is delta != 0?
};

struct ITraceFilter
{
	ITraceFilter(const void* entity) : EntSkip{ entity } { }
	virtual bool ShouldHitEntity(gEntity* pEntity, int) { return pEntity != EntSkip; };
	virtual TraceType_t	GetTraceType() const = 0;
	const void* EntSkip;
};

struct cplane_t
{
	math::Vector	normal;
	float	dist;
	byte	type;			// for fast side tests
	byte	signbits;		// signx + (signy<<1) + (signz<<1)
	byte	pad[2];
};

struct csurface_t
{
	const char* name;
	short		surfaceProps;
	unsigned short	flags;		// BUGBUG: These are declared per surface, not per material, but this database is per-material now
};

// original class name in the sdk: "CBaseTrace"
class trace_t
{
public:
	// Displacement flags tests.
	bool IsDispSurface(void) { return ((dispFlags & DISPSURF_FLAG_SURFACE) != 0); }
	bool IsDispSurfaceWalkable(void) { return ((dispFlags & DISPSURF_FLAG_WALKABLE) != 0); }
	bool IsDispSurfaceBuildable(void) { return ((dispFlags & DISPSURF_FLAG_BUILDABLE) != 0); }
	bool IsDispSurfaceProp1(void) { return ((dispFlags & DISPSURF_FLAG_SURFPROP1) != 0); }
	bool IsDispSurfaceProp2(void) { return ((dispFlags & DISPSURF_FLAG_SURFPROP2) != 0); }
	bool IsDispSurfaceProp3(void) { return ((dispFlags & DISPSURF_FLAG_SURFPROP3) != 0); }
	bool IsDispSurfaceProp4(void) { return ((dispFlags & DISPSURF_FLAG_SURFPROP4) != 0); }

	// Did hit anything at all
	bool DidHit(){
		return fraction < 1.0f || allsolid || startsolid;
	}

public:

	// these members are aligned!!
	math::Vector			startpos;				// start position
	math::Vector			endpos;					// final position
	cplane_t		plane;					// surface normal at impact

	float			fraction;				// time completed, 1.0 = didn't hit anything

	int				contents;				// contents on other side of surface hit
	unsigned short	dispFlags;				// displacement flags for marking surfaces with data

	bool			allsolid;				// if true, plane is not valid
	bool			startsolid;				// if true, the initial point was in a solid area

	float			fractionleftsolid;	// time we left a solid, only valid if we started in solid
	csurface_t		surface;			// surface hit (impact surface)

	int				hitgroup;			// 0 == generic, non-zero is specific body part

	short			physicsbone;		// physics bone hit by trace in studio
	unsigned short	worldSurfaceIndex;	// Index of the msurface2_t, if applicable

	gEntity* pEntity;

	// NOTE: this member is overloaded.
	// If hEnt points at the world entity, then this is the static prop index.
	// Otherwise, this is the hitbox index.
	int			hitbox;					// box hit by trace in studio

private:
	// No copy constructors allowed
	trace_t(const trace_t& vOther);
};

class IEngineTrace
{
public:
	//VIRTUAL_METHOD(void, TraceRay, 5, (const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace), (this, std::cref(ray), fMask, std::cref(pTraceFilter), std::ref(pTrace)));

	void TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) noexcept
	{
		return VirtualMethod::call<void, 5>(this, std::cref(ray), fMask, std::cref(pTraceFilter), std::ref(pTrace));
	}
};