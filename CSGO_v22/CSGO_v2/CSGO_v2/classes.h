#pragma once
#include <Windows.h>
#include <iostream>
#include <math.h>
#include <float.h>

#include "VirtualMethod.h"
#include "math.h"

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

enum CommandButtons : int
{
	IN_ATTACK = (1 << 0),
	IN_JUMP = (1 << 1),
	IN_DUCK = (1 << 2),
	IN_FORWARD = (1 << 3),
	IN_BACK = (1 << 4),
	IN_USE = (1 << 5),
	IN_CANCEL = (1 << 6),
	IN_LEFT = (1 << 7),
	IN_RIGHT = (1 << 8),
	IN_MOVELEFT = (1 << 9),
	IN_MOVERIGHT = (1 << 10),
	IN_SECOND_ATTACK = (1 << 11),
	IN_RUN = (1 << 12),
	IN_RELOAD = (1 << 13),
	IN_LEFT_ALT = (1 << 14),
	IN_RIGHT_ALT = (1 << 15),
	IN_SCORE = (1 << 16),
	IN_SPEED = (1 << 17),
	IN_WALK = (1 << 18),
	IN_ZOOM = (1 << 19),
	IN_FIRST_WEAPON = (1 << 20),
	IN_SECOND_WEAPON = (1 << 21),
	IN_BULLRUSH = (1 << 22),
	IN_FIRST_GRENADE = (1 << 23),
	IN_SECOND_GRENADE = (1 << 24),
	IN_MIDDLE_ATTACK = (1 << 25)
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

class IVDebugOverlay
{
public:
	//virtual int ScreenPosition(const Vector& point, Vector& screen) = 0;
	VIRTUAL_METHOD(int, ScreenPosition, 12, (const math::Vector& point, math::Vector& screen), (this, point, screen))
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
		VIRTUAL_METHOD(const void*, getModel, 8, (), (this + sizeof(uintptr_t)))
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

		math::UtlVector<math::Matrix3x4>& boneCache() noexcept { return *(math::UtlVector<math::Matrix3x4> *)((uintptr_t)this + 0x2914); }
		math::Vector GetBonePos(int bone) noexcept { return boneCache()[bone].GetVecOrgin(); }
};

static_assert(sizeof(gEntity) == 1);

struct CUserCmd
{
	enum
	{
		Button_Attack = 1 << 0,
		Button_Jump = 1 << 1,
		Button_Duck = 1 << 2,
		Button_Forward = 1 << 3,
		Button_Back = 1 << 4,
		Button_Use = 1 << 5,
		Button_MoveLeft = 1 << 9,
		Button_MoveRight = 1 << 10,
		Button_Attack2 = 1 << 11,
		Button_Score = 1 << 16,
		Button_Bullrush = 1 << 22
	};

	void* vmt;
	// For matching server and client commands for debugging
	std::int32_t		command_number;

	// the tick the client created this command
	std::int32_t		tick_count;

	// Player instantaneous view angles.
	math::Vector	viewangles;
	math::Vector	aimdirection;	// For pointing devices. 
	// Intended velocities
	//	forward velocity.
	float	forwardmove;
	//  sideways velocity.
	float	sidemove;
	//  upward velocity.
	float	upmove;
	// Attack button states
	std::int32_t		buttons;
	// Impulse command issued.
	byte    impulse;
	// Current weapon id
	std::int32_t		weaponselect;
	std::int32_t		weaponsubtype;

	std::int32_t		random_seed;	// For shared random functions

	short	mousedx;		// mouse accum in x from create move
	short	mousedy;		// mouse accum in y from create move

	// Client only, tracks whether we've predicted this command at least once
	bool	hasbeenpredicted;
	// TrackIR
	math::Vector headangles;
	math::Vector headoffset;
};
