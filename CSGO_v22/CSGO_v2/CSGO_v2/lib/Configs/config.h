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

// Hotkey activation modes
enum class HotkeyMode : int {
	Hold = 0,    // Active while key is held
	Toggle = 1,  // Press to toggle on/off
	AlwaysOn = 2 // Always active, no key needed
};

struct Hotkey {
	unsigned int virtualKey = 0x0;
	bool searching = false;
	std::string label = "None";
	HotkeyMode mode = HotkeyMode::Hold;

	// Runtime state for toggle mode (not saved)
	bool toggled = false;
	bool lastKeyState = false;

	Hotkey(unsigned int key);
	Hotkey(unsigned int key, HotkeyMode m);

	// Returns true if the hotkey is currently active (respects mode)
	bool isActive();
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
		int AimBone = 0; // 0=Head, 1=Neck, 2=Chest, 3=Stomach

		// Target
		bool FriendlyFire = false;
		bool VisibilityCheck = true;

		// Recoil Control
		bool RCS = false;
		bool StandaloneRCS = true; // RCS works even without aimbot target
		bool SilentRCS = false; // Apply RCS server-side only (no view movement)
		float RCSAmountX = 2.0f; // pitch compensation (2.0 = full)
		float RCSAmountY = 2.0f; // yaw compensation (2.0 = full)
		int RCSStartBullet = 1; // start compensating after N shots
		float RCSSmooth = 1.0f; // 1.0 = instant, higher = smoother application

		// Auto Shoot
		struct AutoShoot {
			bool Enabled = false;
			float FOV = 2.0f;
			int DelayMs = 0; // ms between auto shots, 0 = weapon fire rate
		} autoShoot;

		// Backtracking
		struct Backtrack {
			bool Enabled = false;
			int TimeLimit = 200; // ms (max backtrack window)
			bool DrawTicks = false; // draw backtrack tick dots on enemies
			CfgColor TickColor = CfgColor(255, 255, 100, 180);
		} backtrack;

		// Aimbot Visuals
		bool DrawFov = true;
		CfgColor FovColor = CfgColor(255, 255, 255, 180);
		bool DrawAutoShootFov = false;
		CfgColor AutoShootFovColor = CfgColor(255, 50, 50, 180);
		bool DrawTarget = false;
		CfgColor TargetColor = CfgColor(255, 50, 50);
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
			bool WeaponESP = false;
			int boxType = 0; // eBoxType
			CfgColor color = CfgColor(255,255,255);
		} esp;
		struct Misc
		{
			float AspectRatio = 0.0f;
			bool ThirdPerson = false;
			Hotkey ThirdPersonKey = Hotkey(0x56, HotkeyMode::Toggle); // V key, toggle mode
			float TPDistance = 1.0f;
			float camFOV = 90.0f; // 90 is the default FOV
			bool SteadyCam = false;
			bool NoZoom = false;
			bool NightMode = false;
			float NightModeBrightness = 0.3f; // mat_force_tonemap_scale (lower = darker)
		} misc;
		struct ViewModel
		{
			float ViewModelFOV = 60.0f;
			bool AlwaysDraw = false;
		} viewmodel;
		struct Glow
		{
			bool Enabled = false;
			bool Friendly = false;
			bool LocalPlayer = false;
			bool SyncWithChams = false;
			float Intensity = 0.8f; // Glow alpha/brightness (0.0 - 1.0)
			int Style = 0; // 0=Default, 1=Pulse, 2=Outline, 3=Outline Pulse
			CfgColor EnemyColor = CfgColor(255, 50, 50, 200);
			CfgColor FriendlyColor = CfgColor(50, 150, 255, 200);
			CfgColor LocalColor = CfgColor(50, 255, 50, 200);
		} glow;
		struct Hitmarker
		{
			bool Enabled = false;
			bool ShowDamage = true;
			bool Sound = true;
			float Size = 10.f;    // line length
			float Gap = 4.f;      // gap from center
			float Thickness = 2.f;
			int Duration = 500;   // ms
			CfgColor Color = CfgColor(255, 255, 255);
			CfgColor HeadshotColor = CfgColor(255, 50, 50);
			CfgColor KillColor = CfgColor(255, 200, 50);
		} hitmarker;
		struct Chams
		{
			bool Enabled = false;
			bool Teammates = false;
			bool LocalPlayer = false;
			bool ThroughWalls = true;
			int Style = 0; // 0=Flat, 1=Textured
			float VisibleAlpha = 1.0f;   // Opacity for visible pass
			float InvisibleAlpha = 1.0f; // Opacity for through-walls pass
			CfgColor EnemyVisibleColor = CfgColor(50, 255, 50);
			CfgColor EnemyInvisibleColor = CfgColor(255, 50, 50);
			CfgColor FriendlyVisibleColor = CfgColor(50, 150, 255);
			CfgColor LocalVisibleColor = CfgColor(255, 255, 255);
		} chams;
		struct SkinChanger
		{
			bool Enabled = false;
			int KnifeModel = 0;   // index into knife model list (0 = default)
			int SkinPaintKit = 0; // paint kit ID (0 = default)
			int SkinSeed = 0;     // pattern seed
			float SkinWear = 0.0001f; // wear (0.0 = factory new)
			int StatTrak = -1;    // -1 = disabled
		} skinChanger;
		struct Crosshair
		{
			bool Enabled = false;
			int Style = 0; // 0=Cross, 1=Circle, 2=Dot, 3=Cross+Dot
			float Size = 5.f;
			float Gap = 2.f;
			float Thickness = 1.f;
			bool Outline = true;
			CfgColor Color = CfgColor(0, 255, 0);
			bool RecoilCrosshair = false;
			CfgColor RecoilColor = CfgColor(255, 50, 50);
			bool SniperCrosshair = false;
		} crosshair;
	} visuals;

	struct Misc
	{
		struct Movement
		{
			bool BunnyHop = false;
			bool AirDuck = false;
			bool Strafe = false;
			bool AutoStop = false; // Stop movement when shooting for accuracy
			int AutoStopMode = 0; // 0=All, 1=Manual only, 2=Auto-shoot only
			float AutoStopSpeed = 4.0f; // Deceleration multiplier (higher = faster stop)
		} movement;
		struct Exploits
		{
			bool InfDuck = false; // to do
			bool FakeLag = false;
			int FakeLagAmount = 6; // ticks to choke (1-14)
			bool FakeLagVis = true; // Show server position ghost
		} exploits;
		bool RadarHack = false;
		bool AntiFlash = false;
		float FlashMaxAlpha = 0.f; // 0 = fully remove flash, 255 = normal
		bool SpectatorList = false;
		bool KeybindList = false;
	} misc;

	struct Settings
	{
		bool StreamProof = false;
		bool ShowDebug = false;
		float AnimSpeed = 1.f; // Animations speed
		bool ToggleStyle = true; // true = toggle switch, false = classic checkbox
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

