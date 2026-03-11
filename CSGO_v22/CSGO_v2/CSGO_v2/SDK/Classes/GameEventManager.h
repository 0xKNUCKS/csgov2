#pragma once
#include "SDK/Macros/VirtualMethod.h"

class IGameEvent
{
public:
	// vtable[0] = virtual destructor
	VIRTUAL_METHOD(const char*, GetName, 1, (), (this))
	// [2] IsReliable, [3] IsLocal, [4] IsEmpty
	VIRTUAL_METHOD(bool, GetBool, 5, (const char* key, bool def = false), (this, key, def))
	VIRTUAL_METHOD(int, GetInt, 6, (const char* key, int def = 0), (this, key, def))
	VIRTUAL_METHOD(float, GetFloat, 7, (const char* key, float def = 0.f), (this, key, def))
	VIRTUAL_METHOD(const char*, GetString, 8, (const char* key, const char* def = ""), (this, key, def))
};

class IGameEventListener2
{
public:
	virtual ~IGameEventListener2() {}
	virtual void FireGameEvent(IGameEvent* event) = 0;
	virtual int GetEventDebugID() { return 42; }
};

class IGameEventManager2
{
public:
	VIRTUAL_METHOD(bool, AddListener, 3, (IGameEventListener2* listener, const char* name, bool serverSide), (this, listener, name, serverSide))
	VIRTUAL_METHOD(bool, FindListener, 4, (IGameEventListener2* listener, const char* name), (this, listener, name))
	VIRTUAL_METHOD(void, RemoveListener, 5, (IGameEventListener2* listener), (this, listener))
};
