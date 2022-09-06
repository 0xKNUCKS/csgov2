#pragma once
#include <Windows.h>
#include "Pad.h"
#include "CRC.h"
#include "math.h"

struct CUserCmd
{
	enum
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
	
	uint32_t ComputeCRC() const noexcept;
};


struct VerifiedUserCmd
{
	VerifiedUserCmd(const CUserCmd& cmd) noexcept
	{
		this->cmd = cmd;
		this->crc = cmd.ComputeCRC();
	}

	CUserCmd cmd;
	uint32_t crc;
};