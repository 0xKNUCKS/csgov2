#pragma once
#include <Windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <filesystem>

// Simple color struct to avoid ImGui dependency in config
struct CfgColor {
	float r, g, b, a;
	CfgColor(float r = 1.f, float g = 1.f, float b = 1.f, float a = 1.f) : r(r), g(g), b(b), a(a) {}
	CfgColor(int r, int g, int b, int a = 255)
		: r(r / 255.f), g(g / 255.f), b(b / 255.f), a(a / 255.f) {}
	operator float*() { return &r; }
};

struct Hotkey {
	unsigned int virtualKey = 0x0;
	bool searching = false;
	std::string label = "None";

	Hotkey(unsigned int key);
};

class Config
{
public:

	struct Aimbot
	{
		Hotkey Key = Hotkey(VK_XBUTTON1);
		bool Enabled = false;
		bool Silent = false;
		float FOV = 10.0f;
		float Smooth = 10.0f;
		float SmoothX = 1.0f; // pitch smooth multiplier (1.0 = same as Smooth)
		float SmoothY = 1.0f; // yaw smooth multiplier
		int MaxPlayersInFov = 4;
		bool DrawFov = true;
		bool FriendlyFire = false;
		bool VisibilityCheck = true;
		int AimBone = 0; // 0=Head, 1=Neck, 2=Chest, 3=Stomach

		// Recoil Control
		bool RCS = false;
		bool StandaloneRCS = true; // RCS works even without aimbot target
		float RCSAmountX = 2.0f; // pitch compensation (2.0 = full)
		float RCSAmountY = 2.0f; // yaw compensation (2.0 = full)
		int RCSStartBullet = 1; // start compensating after N shots
		float RCSSmooth = 1.0f; // 1.0 = instant, higher = smoother application

		// Auto shoot
		bool AutoShoot = false;
		float AutoShootFov = 2.0f; // how close aim needs to be to auto-fire
	} aimbot;

	struct Visuals
	{
		bool Enabled = false;
		bool Friendly = false;
		struct ESP
		{
			bool Enabled = false;
			bool Lines = false;
			bool BoundingBox = false;
			bool Skeleton = false;
			bool HealthBar = false;
			bool Name = false;
			bool Dormant = false;
			int boxType = 0; // eBoxType
			CfgColor color = CfgColor(255,255,255);
		} esp;
		struct Misc
		{
			float AspectRatio = 0.0f;
			bool ThirdPerson = false;
			float TPDistance = 1.0f;
			float camFOV = 90.0f; // 90 is the default FOV
			bool SteadyCam = false;
			bool NoZoom = false;
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
			CfgColor Color = CfgColor(94, 156, 255, 255);
			CfgColor SecondColor = CfgColor(255, 255, 255, 255);
		} mouseTracer;
	} settings;

	static std::string GetConfigDir();
	static std::vector<std::string> ListConfigs();
	bool Save(const std::string& name = "default") const;
	bool Load(const std::string& name = "default");
	void Reset();
};

inline Config cfg;

