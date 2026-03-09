#pragma once

#include <Windows.h>

#include "SDK/Macros/VirtualMethod.h"
#include "lib/Math/GameMath.h"
#include "SDK/Models/Model.h"

#ifdef VECTOR_PARANOIA
#define CHECK_VALID( _v)	Assert( (_v).IsValid() )
#else
#ifdef GNUC
#define CHECK_VALID( _v)
#else
#define CHECK_VALID( _v)	0
#endif
#endif

typedef float vec_t;

class IClientNetworkable;
class IClientEntityList;
class IBaseClientDLL;
class RecvProp;
class RecvTable;
class ClientClass;

class IClientNetworkable;
class C_BaseEntity;
class IClientRenderable;
class ICollideable;
class IClientEntity;
class IClientThinkable;
class IClientModelRenderable;
class IClientAlphaProperty;

class gEntity;

// This is the client's version of IUnknown. We may want to use a QueryInterface-like
// mechanism if this gets big.
class IClientUnknown
{
public:
	virtual ICollideable * GetCollideable() = 0;
	virtual IClientNetworkable* GetClientNetworkable() = 0;
	virtual IClientRenderable* GetClientRenderable() = 0;
	virtual IClientEntity* GetIClientEntity() = 0;
	virtual C_BaseEntity* GetBaseEntity() = 0;
	virtual IClientThinkable* GetClientThinkable() = 0;
	virtual IClientAlphaProperty* GetClientAlphaProperty() = 0;
};

class IClientNetworkable
{
public:
	// Gets at the containing class...
	virtual IClientUnknown* GetIClientUnknown() = 0;

	// Called by the engine when the server deletes the entity.
	virtual void			Release() = 0;

	// Supplied automatically by the IMPLEMENT_CLIENTCLASS macros.
	virtual ClientClass* GetClientClass() = 0;

	// This tells the entity what the server says for ShouldTransmit on this entity.
	// Note: This used to be EntityEnteredPVS/EntityRemainedInPVS/EntityLeftPVS.
	virtual void			NotifyShouldTransmit(void* state) = 0;

	virtual void			OnPreDataChanged(void* updateType) = 0;
	virtual void			OnDataChanged(void* updateType) = 0;

	// Called when data is being updated across the network.
	// Only low-level entities should need to know about these.
	virtual void			PreDataUpdate(void* updateType) = 0;
	virtual void			PostDataUpdate(void* updateType) = 0;
	virtual void			OnDataUnchangedInPVS() = 0;


	// Objects become dormant on the client if they leave the PVS on the server.
	virtual bool			IsDormant(void) const = 0;

	// Ent Index is the server handle used to reference this entity.
	// If the index is < 0, that indicates the entity is not known to the server
	virtual int				entindex(void) const = 0;
};

class IClientEntityList
{
public:
    // Get IClientNetworkable interface for specified entity
    virtual IClientNetworkable* GetClientNetworkable(int entnum) = 0;
    virtual void* GetClientNetworkableFromHandle(int hEnt) = 0;
    virtual void* GetClientUnknownFromHandle(int hEnt) = 0;

    // NOTE: This function is only a convenience wrapper.
    // It returns GetClientNetworkable( entnum )->GetIClientEntity().
    virtual gEntity* GetClientEntity(int entnum) = 0;
    virtual void* GetClientEntityFromHandle(int hEnt) = 0;

    // Returns number of entities currently in use
    virtual int                    NumberOfEntities(bool bIncludeNonNetworkable) = 0;

    // Returns highest index actually used
    virtual int                    GetHighestEntityIndex(void) = 0;

    // Sizes entity list to specified size
    virtual void                SetMaxEntities(int maxents) = 0;
    virtual int                    GetMaxEntities() = 0;
};

class IBaseClientDLL
{
public:
	VIRTUAL_METHOD(ClientClass*, GetAllClasses, 8, (), (this))
	VIRTUAL_METHOD(void, CreateMove, 22, (int sequence_number, float input_sample_frametime, bool active), (this, sequence_number, input_sample_frametime, active));
};

class RecvProp
{
public:
    char* m_pVarName;
    void* m_RecvType;
    int                     m_Flags;
    int                     m_StringBufferSize;
    int                     m_bInsideArray;
    const void* m_pExtraData;
    RecvProp* m_pArrayProp;
    void* m_ArrayLengthProxy;
    void* m_ProxyFn;
    void* m_DataTableProxyFn;
    RecvTable* m_pDataTable;
    int                     m_Offset;
    int                     m_ElementStride;
    int                     m_nElements;
    const char* m_pParentArrayPropName;
};

class RecvTable
{
public:
    RecvProp* m_pProps;
    int            m_nProps;
    void* m_pDecoder;
    char* m_pNetTableName;
    bool        m_bInitialized;
    bool        m_bInMainList;
};

class ClientClass
{
public:
    void* m_pCreateFn;
    void* m_pCreateEventFn;
    char* m_pNetworkName;
    RecvTable* m_pRecvTable;
    ClientClass* m_pNext;
    int                m_ClassID;
};

struct OcclusionParams_t
{
	float	m_flMaxOccludeeArea;
	float	m_flMinOccluderArea;
};

class IClientRenderable
{
public:
	// Gets at the containing class...
	virtual IClientUnknown * GetIClientUnknown() = 0;

	// Data accessors
	virtual math::Vector const& GetRenderOrigin(void) = 0;
	virtual math::Vector const& GetRenderAngles(void) = 0;
	virtual bool					ShouldDraw(void) = 0;
	virtual int					    GetRenderFlags(void) = 0; // ERENDERFLAGS_xxx
	virtual void					Unused(void) const {}

	virtual void	GetShadowHandle() const = 0;

	// Used by the leaf system to store its render handle.
	virtual void RenderHandle() = 0;

	// Render baby!
	virtual const void GetModel() const = 0;
	virtual int						DrawModel(int flags, const void* instance) = 0;

	// Get the body parameter
	virtual int		GetBody() = 0;

	// Determine the color modulation amount
	virtual void	GetColorModulation(float* color) = 0;

	// Returns false if the entity shouldn't be drawn due to LOD.
	// (NOTE: This is no longer used/supported, but kept in the vtable for backwards compat)
	virtual bool	LODTest() = 0;

	// Call this to get the current bone transforms for the model.
	// currentTime parameter will affect interpolation
	// nMaxBones specifies how many matrices pBoneToWorldOut can hold. (Should be greater than or
	// equal to studiohdr_t::numbones. Use MAXSTUDIOBONES to be safe.)
	virtual bool	SetupBones(void* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) = 0;

	virtual void	SetupWeights(const void* pBoneToWorld, int nFlexWeightCount, float* pFlexWeights, float* pFlexDelayedWeights) = 0;
	virtual void	DoAnimationEvents(void) = 0;

	// Return this if you want PVS notifications. See IPVSNotify for more info.
	// Note: you must always return the same value from this function. If you don't,
	// undefined things will occur, and they won't be good.
	virtual void* GetPVSNotifyInterface() = 0;

	// Returns the bounds relative to the origin (render bounds)
	virtual void	GetRenderBounds(void* mins, math::Vector& maxs) = 0;

	// returns the bounds as an AABB in worldspace
	virtual void	GetRenderBoundsWorldspace(math::Vector& mins, math::Vector& maxs) = 0;

	// These normally call through to GetRenderAngles/GetRenderBounds, but some entities custom implement them.
	virtual void	GetShadowRenderBounds(math::Vector& mins, math::Vector& maxs, void* shadowType) = 0;

	// Should this object be able to have shadows cast onto it?
	virtual bool	ShouldReceiveProjectedTextures(int flags) = 0;

	// These methods return true if we want a per-renderable shadow cast direction + distance
	virtual bool	GetShadowCastDistance(float* pDist, void* shadowType) const = 0;
	virtual bool	GetShadowCastDirection(math::Vector* pDirection, void* shadowType) const = 0;

	// Other methods related to shadow rendering
	virtual bool	IsShadowDirty() = 0;
	virtual void	MarkShadowDirty(bool bDirty) = 0;

	// Iteration over shadow hierarchy
	virtual IClientRenderable* GetShadowParent() = 0;
	virtual IClientRenderable* FirstShadowChild() = 0;
	virtual IClientRenderable* NextShadowPeer() = 0;

	// Returns the shadow cast type
	virtual void ShadowCastType() = 0;

	virtual void Unused2() {}

	// Create/get/destroy model instance
	virtual void CreateModelInstance() = 0;
	virtual void GetModelInstance() = 0;

	// Returns the transform from RenderOrigin/RenderAngles to world
	virtual const void RenderableToWorldTransform() = 0;

	// Attachments
	virtual int LookupAttachment(const char* pAttachmentName) = 0;
	virtual	bool GetAttachment(int number, math::Vector& origin, math::Vector& angles) = 0;
	virtual bool GetAttachment(int number, void* matrix) = 0;
	virtual bool ComputeLightingOrigin(int nAttachmentIndex, math::Vector modelLightingCenter, const void* matrix, math::Vector& transformedLightingCenter) = 0;

	// Rendering clip plane, should be 4 floats, return value of NULL indicates a disabled render clip plane
	virtual float* GetRenderClipPlane(void) = 0;

	// Get the skin parameter
	virtual int		GetSkin() = 0;

	virtual void	OnThreadedDrawSetup() = 0;

	virtual bool	UsesFlexDelayedWeights() = 0;

	virtual void	RecordToolMessage() = 0;
	virtual bool	ShouldDrawForSplitScreenUser(int nSlot) = 0;

	// NOTE: This is used by renderables to override the default alpha modulation,
	// not including fades, for a renderable. The alpha passed to the function
	// is the alpha computed based on the current renderfx.
	virtual UINT8	OverrideAlphaModulation(UINT8 nAlpha) = 0;

	// NOTE: This is used by renderables to override the default alpha modulation,
	// not including fades, for a renderable's shadow. The alpha passed to the function
	// is the alpha computed based on the current renderfx + any override
	// computed in OverrideAlphaModulation
	virtual UINT8	OverrideShadowAlphaModulation(UINT8 nAlpha) = 0;

	virtual IClientModelRenderable* GetClientModelRenderable() = 0;
};

class IClientThinkable
{
public:
	// Gets at the containing class...
	virtual IClientUnknown * GetIClientUnknown() = 0;

	virtual void				ClientThink() = 0;

	// Called when you're added to the think list.
	// GetThinkHandle's return value must be initialized to INVALID_THINK_HANDLE.
	virtual void	GetThinkHandle() = 0;
	virtual void				SetThinkHandle(void* hThink) = 0;

	// Called by the client when it deletes the entity.
	virtual void				Release() = 0;
};

class  OverlayText_t
{
public:
	OverlayText_t()
	{
		nextOverlayText = 0;
		origin = { 0,0,0 };
		bUseOrigin = false;
		lineOffset = 0;
		flXPos = 0;
		flYPos = 0;
		text[0] = 0;
		m_flEndTime = 0.0f;
		m_nServerCount = -1;
		m_nCreationTick = -1;
		r = g = b = a = 255;
	}

	bool			IsDead() {}
	void			SetEndTime(float duration) {}

	math::Vector			origin;
	bool			bUseOrigin;
	int				lineOffset;
	float			flXPos;
	float			flYPos;
	char			text[512];
	float			m_flEndTime;			// When does this text go away
	int				m_nCreationTick;		// If > 0, show only one server frame
	int				m_nServerCount;			// compare server spawn count to remove stale overlays
	int				r;
	int				g;
	int				b;
	int				a;
	OverlayText_t* nextOverlayText;
};
