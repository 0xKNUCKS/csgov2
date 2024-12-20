#pragma once
#include <Windows.h>
#include "drawing.h"
#include "ESP.h"

class Config
{
public:
	struct Aimbot
	{
		unsigned int Key = VK_XBUTTON1;
		bool Enabled = false;
		bool Silent = false;
		float FOV = 10.0f;
		float Smooth = 10.0f;
		int MaxPlayersInFov = 4;
		bool DrawFov = true;
		bool FriendlyFire = false;
	} aimbot;

	struct Visuals
	{
		bool Enabled = false;
		bool Friendly = false;
		struct ESP
		{
			bool Enabled = false;
			bool Lines = false;
			bool BoudningBox = false;
			bool Skeleton = false;
			bool HealthBar = false;
			int boxType = 0; // eBoxType
			ImColor color = ImColor(255,255,255);
		} esp;
		struct Misc
		{
			float AspectRatio = 0.0f;
			bool ThirdPerson = false;
			float TPDistance = 1.0f;
			float camFOV = 90.0f; // 90 is the default FOV
			bool SteadyCam = false;
			bool noZoon = false;
		} misc;
		struct ViewModel
		{
			float ViewModelFOV = 60.0f;
			bool AlwaysDraw = false;
		} viewmodel;
	} visuals;

	struct Misc
	{
		struct Movement
		{
			bool BunnyHop = false;
			bool AirDuck = false;
			bool Strafe = false;
		} movement;
		struct Exploits
		{
			bool InfDuck = false; // to do
		} exploits;
	} misc;

	struct Settings
	{
		bool StreamProof = false;
		bool ShowDebug = false;
		float AnimSpeed = 1.f; // Animations speed
		struct MouseTracer {
			bool Enabled = true;
			int TrailLength = 40;
			float TrailThickness = 4.f;
			bool AlwaysOn = false;
			ImColor Color = ImColor(94, 156, 255, 255);
			ImColor SecondColor = ImColor(255, 255, 255, 255);
		} mouseTracer;
	} settings;
};

inline Config cfg;

