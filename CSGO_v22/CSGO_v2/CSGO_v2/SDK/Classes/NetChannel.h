#pragma once
#include "SDK/Macros/VirtualMethod.h"

#define FLOW_OUTGOING 0
#define FLOW_INCOMING 1

class INetChannelInfo
{
public:
	VIRTUAL_METHOD(const char*, GetName, 0, (), (this))
	VIRTUAL_METHOD(const char*, GetAddress, 1, (), (this))
	VIRTUAL_METHOD(float, GetTime, 2, (), (this))
	VIRTUAL_METHOD(float, GetTimeConnected, 3, (), (this))
	VIRTUAL_METHOD(int, GetBufferSize, 4, (), (this))
	VIRTUAL_METHOD(int, GetDataRate, 5, (), (this))
	VIRTUAL_METHOD(bool, IsLoopback, 6, (), (this))
	VIRTUAL_METHOD(bool, IsTimingOut, 7, (), (this))
	VIRTUAL_METHOD(bool, IsPlayback, 8, (), (this))
	VIRTUAL_METHOD(float, GetLatency, 9, (int flow), (this, flow))
	VIRTUAL_METHOD(float, GetAvgLatency, 10, (int flow), (this, flow))
	VIRTUAL_METHOD(float, GetAvgLoss, 11, (int flow), (this, flow))
	VIRTUAL_METHOD(float, GetAvgChoke, 12, (int flow), (this, flow))
};
