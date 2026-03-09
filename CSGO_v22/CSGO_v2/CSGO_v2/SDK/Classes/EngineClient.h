#pragma once

#include "SDK/Macros/VirtualMethod.h"
#include "lib/Math/GameMath.h"
#include "SDK/Macros/ButtonCodes.h"
#include "PlayerInfo.h"

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

class ISurface
{
public:
	// virtual void UnlockCursor() = 0; - Index 66
	VIRTUAL_METHOD(void, UnlockCursor, 66, (), (this));
	// virtual void PlaySound(const char *fileName) = 0; - Index 82
	VIRTUAL_METHOD(void, playSoundFile, 82, (const char* fileName), (this, fileName));
};
