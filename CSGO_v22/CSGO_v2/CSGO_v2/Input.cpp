#include "Input.h"

VerifiedUserCmd* CInput::getVerifiedUserCmd(int sequenceNumber) noexcept
{
	return &VerifiedCmds[sequenceNumber % 150];
}
