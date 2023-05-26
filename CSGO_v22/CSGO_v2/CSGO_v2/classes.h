#pragma once
#include <Windows.h>
#include <iostream>
#include <math.h>
#include <float.h>

#include "VirtualMethod.h"
#include "math.h"
#include "Pad.h"
#include "UserCmd.h"
#include "Model.h"

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

enum ClientFrameStage_t
{
	FRAME_UNDEFINED = -1,			// (haven't run any frames yet)
	FRAME_START,

	// A network packet is being recieved
	FRAME_NET_UPDATE_START,
	// Data has been received and we're going to start calling PostDataUpdate
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	// Data has been received and we've called PostDataUpdate on all data recipients
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	// We've received all packets, we can now do interpolation, prediction, etc..
	FRAME_NET_UPDATE_END,

	// We're about to start rendering the scene
	FRAME_RENDER_START,
	// We've finished rendering the scene.
	FRAME_RENDER_END,

	FRAME_NET_FULL_FRAME_UPDATE_ON_REMOVE
};

typedef enum ButtonCode_t
{
	BUTTON_CODE_INVALID = -1,
	BUTTON_CODE_NONE = 0,

	KEY_FIRST = 0,

	KEY_NONE = KEY_FIRST,
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_PAD_0,
	KEY_PAD_1,
	KEY_PAD_2,
	KEY_PAD_3,
	KEY_PAD_4,
	KEY_PAD_5,
	KEY_PAD_6,
	KEY_PAD_7,
	KEY_PAD_8,
	KEY_PAD_9,
	KEY_PAD_DIVIDE,
	KEY_PAD_MULTIPLY,
	KEY_PAD_MINUS,
	KEY_PAD_PLUS,
	KEY_PAD_ENTER,
	KEY_PAD_DECIMAL,
	KEY_LBRACKET,
	KEY_RBRACKET,
	KEY_SEMICOLON,
	KEY_APOSTROPHE,
	KEY_BACKQUOTE,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_BACKSLASH,
	KEY_MINUS,
	KEY_EQUAL,
	KEY_ENTER,
	KEY_SPACE,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_CAPSLOCK,
	KEY_NUMLOCK,
	KEY_ESCAPE,
	KEY_SCROLLLOCK,
	KEY_INSERT,
	KEY_DELETE,
	KEY_HOME,
	KEY_END,
	KEY_PAGEUP,
	KEY_PAGEDOWN,
	KEY_BREAK,
	KEY_LSHIFT,
	KEY_RSHIFT,
	KEY_LALT,
	KEY_RALT,
	KEY_LCONTROL,
	KEY_RCONTROL,
	KEY_LWIN,
	KEY_RWIN,
	KEY_APP,
	KEY_UP,
	KEY_LEFT,
	KEY_DOWN,
	KEY_RIGHT,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_CAPSLOCKTOGGLE,
	KEY_NUMLOCKTOGGLE,
	KEY_SCROLLLOCKTOGGLE,

	KEY_LAST = KEY_SCROLLLOCKTOGGLE,
	KEY_COUNT = KEY_LAST - KEY_FIRST + 1,

	// Mouse
	MOUSE_FIRST = KEY_LAST + 1,

	MOUSE_LEFT = MOUSE_FIRST,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,
	MOUSE_4,
	MOUSE_5,
	MOUSE_WHEEL_UP,		// A fake button which is 'pressed' and 'released' when the wheel is moved up 
	MOUSE_WHEEL_DOWN,	// A fake button which is 'pressed' and 'released' when the wheel is moved down

	MOUSE_LAST = MOUSE_WHEEL_DOWN,
	MOUSE_COUNT = MOUSE_LAST - MOUSE_FIRST + 1,
} ButtonCode_t;


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
    virtual void* GetClientEntity(int entnum) = 0;
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

typedef struct player_info_s
{
	std::uint64_t version;
	union
	{
		std::uint64_t xuid;
		struct
		{
			std::uint32_t xuidLow;
			std::uint32_t xuidHigh;
		};
	};
	// scoreboard information
	char			name[128];
	// local server user ID, unique while server is running
	int				userID;
	// global unique player identifer
	char			guid[33];
	// friends identification number
	UINT32			friendsID;
	// friends name
	char			friendsName[128];
	// true, if player is a bot controlled by game.dll
	bool			fakeplayer;
	// true if player is the HLTV proxy
	bool			ishltv;
#if defined( REPLAY_ENABLED )
	// true if player is the Replay proxy
	bool			isreplay;
#endif
	// custom files CRC for this player
	int			customFiles[4];
	// this counter increases each time the server downloaded a new file
	unsigned char	filesDownloaded;
} player_info_t;

class IVEngineClient
{
public:
	VIRTUAL_METHOD(bool, getPlayerInfo, 8, (int ent_num, player_info_s& pinfo), (this, ent_num, std::ref(pinfo)))
	VIRTUAL_METHOD(int, GetPlayerForUserID, 9, (int userID), (this, userID))
	VIRTUAL_METHOD(bool, Con_IsVisible, 11, (), (this))
	VIRTUAL_METHOD(int, GetLocalPlayerIdx, 12, (), (this)) // GetLocalPlayer index
	VIRTUAL_METHOD(void, GetViewAngles, 18, (math::Vector& va), (this, std::ref(va)))
	VIRTUAL_METHOD(void, SetViewAngles, 19, (const math::Vector& va), (this, std::cref(va)))
	VIRTUAL_METHOD(int, GetMaxClients, 20, (), (this))

	// Given the string pBinding which may be bound to a key, 
	//  returns the string name of the key to which this string is bound. Returns NULL if no such binding exists
	// ex "use" will return its key, which is mostly 'e'. 
	VIRTUAL_METHOD(const char*, Key_LookupBinding, 21, (const char* pBinding), (this, pBinding))
	// Given the name of the key "mouse1", "e", "tab", etc., return the string it is bound to "+jump", "impulse 50", etc.
	VIRTUAL_METHOD(const char*, Key_BindingForKey, 21, (ButtonCode_t code), (this, code))

	// Returns true if the player is fully connected and active in game (i.e, not still loading)
	VIRTUAL_METHOD(bool, IsInGame, 26, (), (this))
	// Returns true if the player is connected, but not necessarily active in game (could still be loading)
	VIRTUAL_METHOD(bool, IsConnected, 27, (), (this))

	// Get Map Name
	VIRTUAL_METHOD(const char*, GetLevelName, 53, (), (this))

	// Execute to CMD ex.(say something) will type in chat something.
	//VIRTUAL_METHOD(void, ExecuteClientCmd, 109, (const char* szCmdString), (this, szCmdString))
	VIRTUAL_METHOD(void, ClientCmdUnrestricted, 114, (const char* cmd, bool fromConsoleOrKeybind = false), (this, cmd, fromConsoleOrKeybind))

	VIRTUAL_METHOD(const math::Matrix4x4&, WorldToScreenMatrix, 37, (), (this))

	auto GetViewAngles() noexcept
	{
		math::Vector ang;
		GetViewAngles(ang);
		return ang;
	}
};

class CInputSystem {
public:
	VIRTUAL_METHOD(void, EnableInput, 11, (bool enable), (this, enable))
	VIRTUAL_METHOD(bool, IsButtonDown, 15, (ButtonCode_t code), (this, code))
	VIRTUAL_METHOD(void, ResetInputState, 39, (), (this))
	VIRTUAL_METHOD(const char*, ButtonCodeToString, 40, (ButtonCode_t code), (this, code)) // use for get Key Buttons
	VIRTUAL_METHOD(ButtonCode_t, VirtualKeyToButtonCode, 44, (int nVirtualKey), (this, nVirtualKey))
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

class Color
{
public:
	// constructors
	Color()
	{
		*((int*)this) = 0;
	}
	Color(int _r, int _g, int _b)
	{
		SetColor(_r, _g, _b, 0);
	}
	Color(int _r, int _g, int _b, int _a)
	{
		SetColor(_r, _g, _b, _a);
	}

	// set the color
	// r - red component (0-255)
	// g - green component (0-255)
	// b - blue component (0-255)
	// a - alpha component, controls transparency (0 - transparent, 255 - opaque);
	void SetColor(int _r, int _g, int _b, int _a = 0)
	{
		_color[0] = (unsigned char)_r;
		_color[1] = (unsigned char)_g;
		_color[2] = (unsigned char)_b;
		_color[3] = (unsigned char)_a;
	}

	void GetColor(int& _r, int& _g, int& _b, int& _a) const
	{
		_r = _color[0];
		_g = _color[1];
		_b = _color[2];
		_a = _color[3];
	}

	void SetRawColor(int color32)
	{
		*((int*)this) = color32;
	}

	int GetRawColor() const
	{
		return *((int*)this);
	}

	inline int r() const { return _color[0]; }
	inline int g() const { return _color[1]; }
	inline int b() const { return _color[2]; }
	inline int a() const { return _color[3]; }

	unsigned char& operator[](int index)
	{
		return _color[index];
	}

	const unsigned char& operator[](int index) const
	{
		return _color[index];
	}

	bool operator == (const Color& rhs) const
	{
		return (*((int*)this) == *((int*)&rhs));
	}

	bool operator != (const Color& rhs) const
	{
		return !(operator==(rhs));
	}

	Color& operator=(const Color& rhs)
	{
		SetRawColor(rhs.GetRawColor());
		return *this;
	}

private:
	unsigned char _color[4];
};

// fuck this shit idek wtf is this, pasted from NEPS. sad.
class gEntity
{
public:
	VIRTUAL_METHOD(void, release, 1, (), (this + sizeof(uintptr_t) * 2))
		VIRTUAL_METHOD(ClientClass*, getClientClass, 2, (), (this + sizeof(uintptr_t) * 2))
		VIRTUAL_METHOD(void, preDataUpdate, 6, (int updateType), (this + sizeof(uintptr_t) * 2, updateType))
		VIRTUAL_METHOD(void, postDataUpdate, 7, (int updateType), (this + sizeof(uintptr_t) * 2, updateType))
		VIRTUAL_METHOD(bool, isDormant, 9, (), (this + sizeof(uintptr_t) * 2))
		VIRTUAL_METHOD(int, index, 10, (), (this + sizeof(uintptr_t) * 2))
		VIRTUAL_METHOD(void, setDestroyedOnRecreateEntities, 13, (), (this + sizeof(uintptr_t) * 2))

		VIRTUAL_METHOD(math::Vector&, getRenderOrigin, 1, (), (this + sizeof(uintptr_t)))
		VIRTUAL_METHOD(bool, shouldDraw, 3, (), (this + sizeof(uintptr_t)))
		VIRTUAL_METHOD(const model_t*, getModel, 8, (), (this + sizeof(uintptr_t)))
		VIRTUAL_METHOD(const math::Matrix3x4&, toWorldTransform, 32, (), (this + sizeof(uintptr_t)))

		VIRTUAL_METHOD(int&, handle, 2, (), (this))
		VIRTUAL_METHOD(void*, getCollideable, 3, (), (this))
		VIRTUAL_METHOD(const math::Vector&, getAbsOrigin, 10, (), (this))
		VIRTUAL_METHOD(const math::Vector&, getAbsAngle, 11, (), (this))
		VIRTUAL_METHOD(void, setModelIndex, 75, (int index), (this, index))
		VIRTUAL_METHOD(int, health, 122, (), (this))
		VIRTUAL_METHOD(bool, isAlive, 156, (), (this))
		VIRTUAL_METHOD(bool, isPlayer, 158, (), (this))
		VIRTUAL_METHOD(bool, isWeapon, 166, (), (this))
		VIRTUAL_METHOD(void, updateClientSideAnimation, 224, (), (this))
		VIRTUAL_METHOD(int, getWeaponSubType, 282, (), (this))
		VIRTUAL_METHOD(void, getObserverMode, 294, (), (this))
		VIRTUAL_METHOD(float, getSpread, 453, (), (this))
		VIRTUAL_METHOD(void, getWeaponType, 455, (), (this))
		VIRTUAL_METHOD(void*, getWeaponData, 461, (), (this))
		VIRTUAL_METHOD(int, getMuzzleAttachmentIndex1stPerson, 468, (gEntity* viewModel), (this, viewModel))
		VIRTUAL_METHOD(int, getMuzzleAttachmentIndex3rdPerson, 469, (), (this))
		VIRTUAL_METHOD(float, getInaccuracy, 483, (), (this))
		VIRTUAL_METHOD(void, updateInaccuracyPenalty, 484, (), (this))

	auto getEyePosition() noexcept
	{
		math::Vector v;
		VirtualMethod::call<void, 285>(this, std::ref(v));
		return v;
	}

	bool SetupBones(math::Matrix3x4* out, int maxBones, int boneMask, float currentTime) noexcept
	{
		return VirtualMethod::call<bool, 13>(this + sizeof(uintptr_t), out, maxBones, boneMask, currentTime);
	}

	math::UtlVector<math::Matrix3x4>& boneCache() noexcept { return *(math::UtlVector<math::Matrix3x4>*)((uintptr_t)this + 0x2914); }
	math::Vector GetBonePosFromChache(int bone) noexcept { return boneCache()[bone].GetVecOrgin(); }
};

static_assert(sizeof(gEntity) == 1);

// this should be in EngineTrace.h but C++ sucks and for some reason give me errors :(

// Contents flags, idfk
#define	CONTENTS_EMPTY			0		// No contents

#define	CONTENTS_SOLID			0x1		// an eye is never valid in a solid
#define	CONTENTS_WINDOW			0x2		// translucent, but not watery (glass)
#define	CONTENTS_AUX			0x4
#define	CONTENTS_GRATE			0x8		// alpha-tested "grate" textures.  Bullets/sight pass through, but solids don't
#define	CONTENTS_SLIME			0x10
#define	CONTENTS_WATER			0x20
#define	CONTENTS_BLOCKLOS		0x40	// block AI line of sight
#define CONTENTS_OPAQUE			0x80	// things that cannot be seen through (may be non-solid though)
#define	LAST_VISIBLE_CONTENTS	CONTENTS_OPAQUE

#define ALL_VISIBLE_CONTENTS (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS-1))

#define CONTENTS_TESTFOGVOLUME	0x100
#define CONTENTS_UNUSED			0x200	

// unused 
// NOTE: If it's visible, grab from the top + update LAST_VISIBLE_CONTENTS
// if not visible, then grab from the bottom.
// CONTENTS_OPAQUE + SURF_NODRAW count as CONTENTS_OPAQUE (shadow-casting toolsblocklight textures)
#define CONTENTS_BLOCKLIGHT		0x400

#define CONTENTS_TEAM1			0x800	// per team contents used to differentiate collisions 
#define CONTENTS_TEAM2			0x1000	// between players and objects on different teams

// ignore CONTENTS_OPAQUE on surfaces that have SURF_NODRAW
#define CONTENTS_IGNORE_NODRAW_OPAQUE	0x2000

// hits entities which are MOVETYPE_PUSH (doors, plats, etc.)
#define CONTENTS_MOVEABLE		0x4000

// remaining contents are non-visible, and don't eat brushes
#define	CONTENTS_AREAPORTAL		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000

#define	CONTENTS_BRUSH_PAINT	0x40000
#define	CONTENTS_GRENADECLIP	0x80000
#define	CONTENTS_UNUSED2		0x100000
#define	CONTENTS_UNUSED3		0x200000
#define	CONTENTS_UNUSED4		0x400000
#define	CONTENTS_UNUSED5		0x800000

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

#define	CONTENTS_MONSTER		0x2000000	// should never be on a brush, only in game
#define	CONTENTS_DEBRIS			0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000
#define CONTENTS_HITBOX			0x40000000	// use accurate hitboxes on trace

// ray_t flags
#define	MASK_ALL					(0xFFFFFFFF)
// everything that is normally solid
#define	MASK_SOLID					(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
// everything that blocks player movement
#define	MASK_PLAYERSOLID			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
// blocks npc movement
#define	MASK_NPCSOLID				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
// blocks fluid movement
#define	MASK_NPCFLUID				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
// water physics in these contents
#define	MASK_WATER					(CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME)
// everything that blocks lighting
#define	MASK_OPAQUE					(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE)
// everything that blocks lighting, but with monsters added.
#define MASK_OPAQUE_AND_NPCS		(MASK_OPAQUE|CONTENTS_MONSTER)
// everything that blocks line of sight for AI
#define MASK_BLOCKLOS				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_BLOCKLOS)
// everything that blocks line of sight for AI plus NPCs
#define MASK_BLOCKLOS_AND_NPCS		(MASK_BLOCKLOS|CONTENTS_MONSTER)
// everything that blocks line of sight for players
#define	MASK_VISIBLE					(MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE)
// everything that blocks line of sight for players, but with monsters added.
#define MASK_VISIBLE_AND_NPCS		(MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE)
// bullets see these as solid
#define	MASK_SHOT					(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX)
// for finding floor height
#define MASK_FLOORTRACE				(CONTENTS_SOLID|CONTENTS_MOVEABLE|                 CONTENTS_WINDOW|CONTENTS_DEBRIS)
// for csgo weapon clipping
#define MASK_WEAPONCLIPPING			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS)
// bullets see these as solid, except monsters (world+brush only)
#define MASK_SHOT_BRUSHONLY			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS)
// non-raycasted weapons see this as solid (includes grates)
#define MASK_SHOT_HULL				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE)
// hits solids (not grates) and passes through everything else
#define MASK_SHOT_PORTAL			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER)
// everything normally solid, except monsters (world+brush only)
#define MASK_SOLID_BRUSHONLY		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE)
// everything normally solid for player movement, except monsters (world+brush only)
#define MASK_PLAYERSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE)
// everything normally solid for npc movement, except monsters (world+brush only)
#define MASK_NPCSOLID_BRUSHONLY		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
// just the world, used for route rebuilding
#define MASK_NPCWORLDSTATIC			(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
// just the world, used for route rebuilding
#define MASK_NPCWORLDSTATIC_FLUID	(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP)
// These are things that can split areaportals
#define MASK_SPLITAREAPORTAL		(CONTENTS_WATER|CONTENTS_SLIME)

// everything that blocks corpse movement
// UNDONE: Not used yet / may be deleted
#define	MASK_DEADSOLID				(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_GRATE)

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
		m_IsSwept = m_Delta.x || m_Delta.y || m_Delta.z;
	}

	math::Vector  m_Start;	// starting point, centered within the extents
	math::Vector  m_Delta;	// direction + length of the ray
	math::Vector  m_StartOffset;	// Add this to m_Start to get the actual ray start
	math::Vector  m_Extents;	// Describes an axis aligned box extruded along a ray
	const math::Matrix3x4* m_pWorldAxisTransform;
	bool	m_IsRay;	// are the extents zero?
	bool	m_IsSwept;	// is delta != 0?
};

class ITraceFilter
{
public:
	ITraceFilter() {}
	ITraceFilter(const void* entity) { EntSkip = entity; }
	virtual bool ShouldHitEntity(gEntity* pEntity, int) { return pEntity != EntSkip; };
	//TraceType_t	GetTraceType();
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
	bool DidHit() {
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

	trace_t() {};

private:
	// No copy constructors allowed
	trace_t(const trace_t& vOther);
};

class IEngineTrace
{
public:
	VIRTUAL_METHOD(void, TraceRay, 5, (const Ray_t& ray, unsigned int fMask, ITraceFilter& pTraceFilter, trace_t& pTrace), (this, std::cref(ray), fMask, std::cref(pTraceFilter), std::ref(pTrace)));

	//void TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter& pTraceFilter, trace_t& pTrace) noexcept
	//{
	//	return VirtualMethod::call<void, 5>(this, std::cref(ray), fMask, std::cref(pTraceFilter), std::ref(pTrace));
	//}
};

class ConVar
{
public:
	VIRTUAL_METHOD(float, GetFloat, 12, (), (this))
	VIRTUAL_METHOD(int, GetInt, 13, (), (this))
	VIRTUAL_METHOD(void, SetValue, 14, (const char* value), (this, value))
	VIRTUAL_METHOD(void, SetValue, 15, (float value), (this, value))
	VIRTUAL_METHOD(void, SetValue, 16, (int value), (this, value))
};

class ICvar
{
public:
	// virtual const ConVar	*FindVar ( const char *var_name ) const = 0;
	VIRTUAL_METHOD(ConVar*, FindVar, 15, (const char* var_name), (this, var_name));
};