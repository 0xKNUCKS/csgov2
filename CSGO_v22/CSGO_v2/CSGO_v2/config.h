#pragma once
#include <Windows.h>
#include "drawing.h"

class Config
{
public:
	struct Aimbot
	{
		unsigned int Key = VK_XBUTTON1;
		bool Enabled = false;
		float FOV = 10.0f;
		float Smooth = 10.0f;
		bool DrawFov = true;
		bool FriendlyFire = false;
	} aimbot;

	struct Visuals
	{
		bool Enabled = false;
		struct ESP
		{
			bool Enabled = false;
			//eBoxType boxType = eBoxType::Outlined;
			ImColor color = ImColor(255,255,255);
		} esp;
	} visuals;

	struct Misc
	{
		struct Movement
		{
			bool BunnyHop = false;
			unsigned int bhopMode = 1;
		} movement;
	} misc;

	struct Settings
	{
		bool StreamProof = false;
	} settings;
};

inline Config cfg;

