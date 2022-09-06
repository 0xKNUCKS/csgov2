#pragma once
#include "VirtualMethod.h"
#include "UserCmd.h"

class CInput
{
public:
	VIRTUAL_METHOD(CUserCmd*, getUserCmd, 8, (int slot, int sequenceNumber), (this, slot, sequenceNumber))
	VerifiedUserCmd* getVerifiedUserCmd(int sequenceNumber) noexcept;

	PAD(12)
		bool isTrackIRAvailable;
	bool isMouseInitialized;
	bool isMouseActive;
	PAD(154)
		bool isCameraInThirdPerson;
	PAD(2)
		math::Vector cameraOffset;
	PAD(56)
	CUserCmd* cmds;
	VerifiedUserCmd* VerifiedCmds;
};