#include "config.h"
#include "lib/utils/utils.h"
#include "MD5.h"

Hotkey::Hotkey(unsigned int key)
	: virtualKey(key)
	, label(utils::VirtualKeyToString(key))
{
}

Hotkey::Hotkey(unsigned int key, HotkeyMode m)
	: virtualKey(key)
	, label(utils::VirtualKeyToString(key))
	, mode(m)
{
}

bool Hotkey::isActive()
{
	if (mode == HotkeyMode::AlwaysOn)
		return true;

	bool keyDown = virtualKey != 0 && (GetAsyncKeyState(virtualKey) & 0x8000) != 0;

	if (mode == HotkeyMode::Toggle) {
		// Rising edge detection
		if (keyDown && !lastKeyState)
			toggled = !toggled;
		lastKeyState = keyDown;
		return toggled;
	}

	// Hold mode
	return keyDown;
}

// Config directory lives next to the log directory
std::string Config::GetConfigDir()
{
	std::string dir = "C:\\Users\\adama\\Documents\\CSGO_v2_Configs";
	std::filesystem::create_directories(dir);
	return dir;
}

// Forward declare - defined after Save
static bool VerifyConfig(const std::string& path);

std::vector<std::string> Config::ListConfigs()
{
	std::vector<std::string> configs;
	std::string dir = GetConfigDir();
	for (auto& entry : std::filesystem::directory_iterator(dir)) {
		if (entry.path().extension() == ".cfg" && VerifyConfig(entry.path().string()))
			configs.push_back(entry.path().stem().string());
	}
	return configs;
}

// Simple key=value writer. One section per category, flat within.
// First line is a header with MD5 checksum of the config body.
bool Config::Save(const std::string& name) const
{
	std::string path = GetConfigDir() + "\\" + name + ".cfg";

	// Build config body into a string first so we can hash it
	std::ostringstream body;

	body << "[aimbot]\n";
	body << "Key=" << aimbot.Key.virtualKey << "\n";
	body << "KeyMode=" << static_cast<int>(aimbot.Key.mode) << "\n";
	body << "Enabled=" << aimbot.Enabled << "\n";
	body << "Silent=" << aimbot.Silent << "\n";
	body << "FOV=" << aimbot.FOV << "\n";
	body << "Smooth=" << aimbot.Smooth << "\n";
	body << "SmoothX=" << aimbot.SmoothX << "\n";
	body << "SmoothY=" << aimbot.SmoothY << "\n";
	body << "MaxPlayersInFov=" << aimbot.MaxPlayersInFov << "\n";
	body << "FriendlyFire=" << aimbot.FriendlyFire << "\n";
	body << "VisibilityCheck=" << aimbot.VisibilityCheck << "\n";
	body << "AimBone=" << aimbot.AimBone << "\n";
	body << "RCS=" << aimbot.RCS << "\n";
	body << "StandaloneRCS=" << aimbot.StandaloneRCS << "\n";
	body << "SilentRCS=" << aimbot.SilentRCS << "\n";
	body << "RCSAmountX=" << aimbot.RCSAmountX << "\n";
	body << "RCSAmountY=" << aimbot.RCSAmountY << "\n";
	body << "RCSStartBullet=" << aimbot.RCSStartBullet << "\n";
	body << "RCSSmooth=" << aimbot.RCSSmooth << "\n";
	body << "AutoShootEnabled=" << aimbot.autoShoot.Enabled << "\n";
	body << "AutoShootFov=" << aimbot.autoShoot.FOV << "\n";
	body << "AutoShootDelay=" << aimbot.autoShoot.DelayMs << "\n";
	body << "BacktrackEnabled=" << aimbot.backtrack.Enabled << "\n";
	body << "BacktrackTimeLimit=" << aimbot.backtrack.TimeLimit << "\n";
	body << "BacktrackDrawTicks=" << aimbot.backtrack.DrawTicks << "\n";
	body << "BacktrackTickColor=" << aimbot.backtrack.TickColor.r << "," << aimbot.backtrack.TickColor.g << ","
	  << aimbot.backtrack.TickColor.b << "," << aimbot.backtrack.TickColor.a << "\n";
	body << "DrawFov=" << aimbot.DrawFov << "\n";
	body << "FovColor=" << aimbot.FovColor.r << "," << aimbot.FovColor.g << ","
	  << aimbot.FovColor.b << "," << aimbot.FovColor.a << "\n";
	body << "DrawAutoShootFov=" << aimbot.DrawAutoShootFov << "\n";
	body << "AutoShootFovColor=" << aimbot.AutoShootFovColor.r << "," << aimbot.AutoShootFovColor.g << ","
	  << aimbot.AutoShootFovColor.b << "," << aimbot.AutoShootFovColor.a << "\n";
	body << "DrawTarget=" << aimbot.DrawTarget << "\n";
	body << "TargetColor=" << aimbot.TargetColor.r << "," << aimbot.TargetColor.g << ","
	  << aimbot.TargetColor.b << "," << aimbot.TargetColor.a << "\n";

	body << "\n[visuals]\n";
	body << "Enabled=" << visuals.Enabled << "\n";
	body << "Friendly=" << visuals.Friendly << "\n";

	body << "\n[visuals.esp]\n";
	body << "Enabled=" << visuals.esp.Enabled << "\n";
	body << "Lines=" << visuals.esp.Lines << "\n";
	body << "BoundingBox=" << visuals.esp.BoundingBox << "\n";
	body << "Skeleton=" << visuals.esp.Skeleton << "\n";
	body << "HealthBar=" << visuals.esp.HealthBar << "\n";
	body << "Name=" << visuals.esp.Name << "\n";
	body << "Dormant=" << visuals.esp.Dormant << "\n";
	body << "WeaponESP=" << visuals.esp.WeaponESP << "\n";
	body << "BoxType=" << visuals.esp.boxType << "\n";
	body << "Color=" << visuals.esp.color.r << "," << visuals.esp.color.g << ","
	  << visuals.esp.color.b << "," << visuals.esp.color.a << "\n";
	body << "BoxColor=" << visuals.esp.BoxColor.r << "," << visuals.esp.BoxColor.g << ","
	  << visuals.esp.BoxColor.b << "," << visuals.esp.BoxColor.a << "\n";
	body << "SkeletonColor=" << visuals.esp.SkeletonColor.r << "," << visuals.esp.SkeletonColor.g << ","
	  << visuals.esp.SkeletonColor.b << "," << visuals.esp.SkeletonColor.a << "\n";
	body << "NameColor=" << visuals.esp.NameColor.r << "," << visuals.esp.NameColor.g << ","
	  << visuals.esp.NameColor.b << "," << visuals.esp.NameColor.a << "\n";
	body << "SnapLineColor=" << visuals.esp.SnapLineColor.r << "," << visuals.esp.SnapLineColor.g << ","
	  << visuals.esp.SnapLineColor.b << "," << visuals.esp.SnapLineColor.a << "\n";
	body << "WeaponColor=" << visuals.esp.WeaponColor.r << "," << visuals.esp.WeaponColor.g << ","
	  << visuals.esp.WeaponColor.b << "," << visuals.esp.WeaponColor.a << "\n";

	body << "\n[visuals.misc]\n";
	body << "AspectRatio=" << visuals.misc.AspectRatio << "\n";
	body << "ThirdPerson=" << visuals.misc.ThirdPerson << "\n";
	body << "TPKey=" << visuals.misc.ThirdPersonKey.virtualKey << "\n";
	body << "TPKeyMode=" << static_cast<int>(visuals.misc.ThirdPersonKey.mode) << "\n";
	body << "TPDistance=" << visuals.misc.TPDistance << "\n";
	body << "CamFOV=" << visuals.misc.camFOV << "\n";
	body << "SteadyCam=" << visuals.misc.SteadyCam << "\n";
	body << "NoZoom=" << visuals.misc.NoZoom << "\n";
	body << "NightMode=" << visuals.misc.NightMode << "\n";
	body << "NightModeBrightness=" << visuals.misc.NightModeBrightness << "\n";

	body << "\n[visuals.viewmodel]\n";
	body << "ViewModelFOV=" << visuals.viewmodel.ViewModelFOV << "\n";
	body << "AlwaysDraw=" << visuals.viewmodel.AlwaysDraw << "\n";

	body << "\n[visuals.glow]\n";
	body << "Enabled=" << visuals.glow.Enabled << "\n";
	body << "Friendly=" << visuals.glow.Friendly << "\n";
	body << "LocalPlayer=" << visuals.glow.LocalPlayer << "\n";
	body << "SyncWithChams=" << visuals.glow.SyncWithChams << "\n";
	body << "Intensity=" << visuals.glow.Intensity << "\n";
	body << "Style=" << visuals.glow.Style << "\n";
	body << "EnemyColor=" << visuals.glow.EnemyColor.r << "," << visuals.glow.EnemyColor.g << ","
	  << visuals.glow.EnemyColor.b << "," << visuals.glow.EnemyColor.a << "\n";
	body << "FriendlyColor=" << visuals.glow.FriendlyColor.r << "," << visuals.glow.FriendlyColor.g << ","
	  << visuals.glow.FriendlyColor.b << "," << visuals.glow.FriendlyColor.a << "\n";
	body << "LocalColor=" << visuals.glow.LocalColor.r << "," << visuals.glow.LocalColor.g << ","
	  << visuals.glow.LocalColor.b << "," << visuals.glow.LocalColor.a << "\n";

	body << "\n[visuals.hitmarker]\n";
	body << "Enabled=" << visuals.hitmarker.Enabled << "\n";
	body << "ShowDamage=" << visuals.hitmarker.ShowDamage << "\n";
	body << "Sound=" << visuals.hitmarker.Sound << "\n";
	body << "Size=" << visuals.hitmarker.Size << "\n";
	body << "Gap=" << visuals.hitmarker.Gap << "\n";
	body << "Thickness=" << visuals.hitmarker.Thickness << "\n";
	body << "Duration=" << visuals.hitmarker.Duration << "\n";
	body << "Color=" << visuals.hitmarker.Color.r << "," << visuals.hitmarker.Color.g << ","
	  << visuals.hitmarker.Color.b << "," << visuals.hitmarker.Color.a << "\n";
	body << "HeadshotColor=" << visuals.hitmarker.HeadshotColor.r << "," << visuals.hitmarker.HeadshotColor.g << ","
	  << visuals.hitmarker.HeadshotColor.b << "," << visuals.hitmarker.HeadshotColor.a << "\n";
	body << "KillColor=" << visuals.hitmarker.KillColor.r << "," << visuals.hitmarker.KillColor.g << ","
	  << visuals.hitmarker.KillColor.b << "," << visuals.hitmarker.KillColor.a << "\n";

	body << "\n[visuals.chams]\n";
	body << "Enabled=" << visuals.chams.Enabled << "\n";
	body << "Teammates=" << visuals.chams.Teammates << "\n";
	body << "LocalPlayer=" << visuals.chams.LocalPlayer << "\n";
	body << "ThroughWalls=" << visuals.chams.ThroughWalls << "\n";
	body << "Style=" << visuals.chams.Style << "\n";
	body << "VisibleAlpha=" << visuals.chams.VisibleAlpha << "\n";
	body << "InvisibleAlpha=" << visuals.chams.InvisibleAlpha << "\n";
	body << "EnemyVisibleColor=" << visuals.chams.EnemyVisibleColor.r << "," << visuals.chams.EnemyVisibleColor.g << ","
	  << visuals.chams.EnemyVisibleColor.b << "," << visuals.chams.EnemyVisibleColor.a << "\n";
	body << "EnemyInvisibleColor=" << visuals.chams.EnemyInvisibleColor.r << "," << visuals.chams.EnemyInvisibleColor.g << ","
	  << visuals.chams.EnemyInvisibleColor.b << "," << visuals.chams.EnemyInvisibleColor.a << "\n";
	body << "FriendlyVisibleColor=" << visuals.chams.FriendlyVisibleColor.r << "," << visuals.chams.FriendlyVisibleColor.g << ","
	  << visuals.chams.FriendlyVisibleColor.b << "," << visuals.chams.FriendlyVisibleColor.a << "\n";
	body << "LocalVisibleColor=" << visuals.chams.LocalVisibleColor.r << "," << visuals.chams.LocalVisibleColor.g << ","
	  << visuals.chams.LocalVisibleColor.b << "," << visuals.chams.LocalVisibleColor.a << "\n";

	body << "\n[visuals.skinChanger]\n";
	body << "Enabled=" << visuals.skinChanger.Enabled << "\n";
	body << "KnifeModel=" << visuals.skinChanger.KnifeModel << "\n";
	body << "SkinPaintKit=" << visuals.skinChanger.SkinPaintKit << "\n";
	body << "SkinSeed=" << visuals.skinChanger.SkinSeed << "\n";
	body << "SkinWear=" << visuals.skinChanger.SkinWear << "\n";
	body << "StatTrak=" << visuals.skinChanger.StatTrak << "\n";

	body << "\n[visuals.crosshair]\n";
	body << "Enabled=" << visuals.crosshair.Enabled << "\n";
	body << "Style=" << visuals.crosshair.Style << "\n";
	body << "Size=" << visuals.crosshair.Size << "\n";
	body << "Gap=" << visuals.crosshair.Gap << "\n";
	body << "Thickness=" << visuals.crosshair.Thickness << "\n";
	body << "Outline=" << visuals.crosshair.Outline << "\n";
	body << "Color=" << visuals.crosshair.Color.r << "," << visuals.crosshair.Color.g << ","
	  << visuals.crosshair.Color.b << "," << visuals.crosshair.Color.a << "\n";
	body << "RecoilCrosshair=" << visuals.crosshair.RecoilCrosshair << "\n";
	body << "RecoilColor=" << visuals.crosshair.RecoilColor.r << "," << visuals.crosshair.RecoilColor.g << ","
	  << visuals.crosshair.RecoilColor.b << "," << visuals.crosshair.RecoilColor.a << "\n";
	body << "SniperCrosshair=" << visuals.crosshair.SniperCrosshair << "\n";

	body << "\n[misc]\n";
	body << "RadarHack=" << misc.RadarHack << "\n";
	body << "AntiFlash=" << misc.AntiFlash << "\n";
	body << "FlashMaxAlpha=" << misc.FlashMaxAlpha << "\n";
	body << "SpectatorList=" << misc.SpectatorList << "\n";
	body << "KeybindList=" << misc.KeybindList << "\n";

	body << "\n[misc.movement]\n";
	body << "BunnyHop=" << misc.movement.BunnyHop << "\n";
	body << "AirDuck=" << misc.movement.AirDuck << "\n";
	body << "Strafe=" << misc.movement.Strafe << "\n";
	body << "AutoStop=" << misc.movement.AutoStop << "\n";
	body << "AutoStopMode=" << misc.movement.AutoStopMode << "\n";
	body << "AutoStopSpeed=" << misc.movement.AutoStopSpeed << "\n";

	body << "\n[misc.exploits]\n";
	body << "InfDuck=" << misc.exploits.InfDuck << "\n";
	body << "FakeLag=" << misc.exploits.FakeLag << "\n";
	body << "FakeLagAmount=" << misc.exploits.FakeLagAmount << "\n";
	body << "FakeLagVis=" << misc.exploits.FakeLagVis << "\n";

	body << "\n[settings]\n";
	body << "StreamProof=" << settings.StreamProof << "\n";
	body << "ShowDebug=" << settings.ShowDebug << "\n";
	body << "AnimSpeed=" << settings.AnimSpeed << "\n";
	body << "ToggleStyle=" << settings.ToggleStyle << "\n";

	body << "\n[settings.mouseTracer]\n";
	body << "Enabled=" << settings.mouseTracer.Enabled << "\n";
	body << "TrailLength=" << settings.mouseTracer.TrailLength << "\n";
	body << "TrailThickness=" << settings.mouseTracer.TrailThickness << "\n";
	body << "AlwaysOn=" << settings.mouseTracer.AlwaysOn << "\n";
	body << "Color=" << settings.mouseTracer.Color.r << "," << settings.mouseTracer.Color.g << ","
	  << settings.mouseTracer.Color.b << "," << settings.mouseTracer.Color.a << "\n";
	body << "SecondColor=" << settings.mouseTracer.SecondColor.r << "," << settings.mouseTracer.SecondColor.g << ","
	  << settings.mouseTracer.SecondColor.b << "," << settings.mouseTracer.SecondColor.a << "\n";

	std::string bodyStr = body.str();
	std::string hash = md5(bodyStr);

	// Write header + body
	std::ofstream f(path);
	if (!f.is_open())
		return false;

	f << "# CSGO_v2 cfg " << hash << "\n";
	f << bodyStr;

	return f.good();
}

// Header format: "# CSGO_v2 cfg <md5hash>"
static const std::string CFG_HEADER_PREFIX = "# CSGO_v2 cfg ";

// Verify a config file's integrity. Returns true if header + hash match body.
static bool VerifyConfig(const std::string& path)
{
	std::ifstream f(path);
	if (!f.is_open()) return false;

	// Read header line
	std::string header;
	if (!std::getline(f, header)) return false;

	// Trim \r
	while (!header.empty() && header.back() == '\r')
		header.pop_back();

	// Check prefix
	if (header.substr(0, CFG_HEADER_PREFIX.size()) != CFG_HEADER_PREFIX)
		return false;

	std::string expectedHash = header.substr(CFG_HEADER_PREFIX.size());
	if (expectedHash.size() != 32) return false; // MD5 is always 32 hex chars

	// Read the rest of the file (body)
	std::string body((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	std::string actualHash = md5(body);

	return actualHash == expectedHash;
}

// Parse helpers
namespace {
	// Parses the INI body (skips the header line)
	std::unordered_map<std::string, std::string> ParseINI(const std::string& path)
	{
		std::unordered_map<std::string, std::string> kv;
		std::ifstream f(path);
		if (!f.is_open()) return kv;

		std::string line, section;
		bool firstLine = true;
		while (std::getline(f, line)) {
			// Skip header line
			if (firstLine) { firstLine = false; continue; }

			// Trim whitespace
			while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
				line.pop_back();
			if (line.empty()) continue;

			if (line.front() == '[' && line.back() == ']') {
				section = line.substr(1, line.size() - 2);
				continue;
			}

			auto eq = line.find('=');
			if (eq == std::string::npos) continue;

			std::string key = section + "." + line.substr(0, eq);
			std::string val = line.substr(eq + 1);
			kv[key] = val;
		}
		return kv;
	}

	bool GetBool(const std::unordered_map<std::string, std::string>& kv, const std::string& key, bool fallback)
	{
		auto it = kv.find(key);
		return it != kv.end() ? (it->second == "1" || it->second == "true") : fallback;
	}

	float GetFloat(const std::unordered_map<std::string, std::string>& kv, const std::string& key, float fallback)
	{
		auto it = kv.find(key);
		if (it == kv.end()) return fallback;
		try { return std::stof(it->second); }
		catch (...) { return fallback; }
	}

	int GetInt(const std::unordered_map<std::string, std::string>& kv, const std::string& key, int fallback)
	{
		auto it = kv.find(key);
		if (it == kv.end()) return fallback;
		try { return std::stoi(it->second); }
		catch (...) { return fallback; }
	}

	CfgColor GetColor(const std::unordered_map<std::string, std::string>& kv, const std::string& key, CfgColor fallback)
	{
		auto it = kv.find(key);
		if (it == kv.end()) return fallback;
		CfgColor c;
		if (sscanf_s(it->second.c_str(), "%f,%f,%f,%f", &c.r, &c.g, &c.b, &c.a) == 4)
			return c;
		return fallback;
	}
}

bool Config::Load(const std::string& name)
{
	std::string path = GetConfigDir() + "\\" + name + ".cfg";
	if (!std::filesystem::exists(path))
		return false;

	if (!VerifyConfig(path))
		return false;

	auto kv = ParseINI(path);
	if (kv.empty())
		return false;

	Config defaults;

	// Aimbot
	unsigned int key = (unsigned int)GetInt(kv, "aimbot.Key", defaults.aimbot.Key.virtualKey);
	aimbot.Key = Hotkey(key);
	aimbot.Key.mode = static_cast<HotkeyMode>(GetInt(kv, "aimbot.KeyMode", static_cast<int>(defaults.aimbot.Key.mode)));
	aimbot.Enabled      = GetBool(kv, "aimbot.Enabled", defaults.aimbot.Enabled);
	aimbot.Silent        = GetBool(kv, "aimbot.Silent", defaults.aimbot.Silent);
	aimbot.FOV           = GetFloat(kv, "aimbot.FOV", defaults.aimbot.FOV);
	aimbot.Smooth        = GetFloat(kv, "aimbot.Smooth", defaults.aimbot.Smooth);
	aimbot.MaxPlayersInFov = GetInt(kv, "aimbot.MaxPlayersInFov", defaults.aimbot.MaxPlayersInFov);
	aimbot.DrawFov       = GetBool(kv, "aimbot.DrawFov", defaults.aimbot.DrawFov);
	aimbot.FriendlyFire  = GetBool(kv, "aimbot.FriendlyFire", defaults.aimbot.FriendlyFire);
	aimbot.SmoothX       = GetFloat(kv, "aimbot.SmoothX", defaults.aimbot.SmoothX);
	aimbot.SmoothY       = GetFloat(kv, "aimbot.SmoothY", defaults.aimbot.SmoothY);
	aimbot.VisibilityCheck = GetBool(kv, "aimbot.VisibilityCheck", defaults.aimbot.VisibilityCheck);
	aimbot.AimBone       = GetInt(kv, "aimbot.AimBone", defaults.aimbot.AimBone);
	aimbot.RCS           = GetBool(kv, "aimbot.RCS", defaults.aimbot.RCS);
	aimbot.StandaloneRCS = GetBool(kv, "aimbot.StandaloneRCS", defaults.aimbot.StandaloneRCS);
	aimbot.SilentRCS     = GetBool(kv, "aimbot.SilentRCS", defaults.aimbot.SilentRCS);
	aimbot.RCSAmountX    = GetFloat(kv, "aimbot.RCSAmountX", defaults.aimbot.RCSAmountX);
	aimbot.RCSAmountY    = GetFloat(kv, "aimbot.RCSAmountY", defaults.aimbot.RCSAmountY);
	aimbot.RCSStartBullet = GetInt(kv, "aimbot.RCSStartBullet", defaults.aimbot.RCSStartBullet);
	aimbot.RCSSmooth     = GetFloat(kv, "aimbot.RCSSmooth", defaults.aimbot.RCSSmooth);
	// Auto Shoot (support both old "AutoShoot" and new "AutoShootEnabled" keys)
	aimbot.autoShoot.Enabled = GetBool(kv, "aimbot.AutoShootEnabled",
		GetBool(kv, "aimbot.AutoShoot", defaults.aimbot.autoShoot.Enabled));
	aimbot.autoShoot.FOV     = GetFloat(kv, "aimbot.AutoShootFov", defaults.aimbot.autoShoot.FOV);
	aimbot.autoShoot.DelayMs = GetInt(kv, "aimbot.AutoShootDelay", defaults.aimbot.autoShoot.DelayMs);
	// Backtrack
	aimbot.backtrack.Enabled   = GetBool(kv, "aimbot.BacktrackEnabled", defaults.aimbot.backtrack.Enabled);
	aimbot.backtrack.TimeLimit = GetInt(kv, "aimbot.BacktrackTimeLimit", defaults.aimbot.backtrack.TimeLimit);
	aimbot.backtrack.DrawTicks = GetBool(kv, "aimbot.BacktrackDrawTicks", defaults.aimbot.backtrack.DrawTicks);
	aimbot.backtrack.TickColor = GetColor(kv, "aimbot.BacktrackTickColor", defaults.aimbot.backtrack.TickColor);
	// Aimbot Visuals
	aimbot.DrawFov       = GetBool(kv, "aimbot.DrawFov", defaults.aimbot.DrawFov);
	aimbot.FovColor      = GetColor(kv, "aimbot.FovColor", defaults.aimbot.FovColor);
	aimbot.DrawAutoShootFov = GetBool(kv, "aimbot.DrawAutoShootFov", defaults.aimbot.DrawAutoShootFov);
	aimbot.AutoShootFovColor = GetColor(kv, "aimbot.AutoShootFovColor", defaults.aimbot.AutoShootFovColor);
	aimbot.DrawTarget    = GetBool(kv, "aimbot.DrawTarget", defaults.aimbot.DrawTarget);
	aimbot.TargetColor   = GetColor(kv, "aimbot.TargetColor", defaults.aimbot.TargetColor);

	// Visuals
	visuals.Enabled  = GetBool(kv, "visuals.Enabled", defaults.visuals.Enabled);
	visuals.Friendly = GetBool(kv, "visuals.Friendly", defaults.visuals.Friendly);

	// Visuals ESP
	visuals.esp.Enabled     = GetBool(kv, "visuals.esp.Enabled", defaults.visuals.esp.Enabled);
	visuals.esp.Lines       = GetBool(kv, "visuals.esp.Lines", defaults.visuals.esp.Lines);
	visuals.esp.BoundingBox = GetBool(kv, "visuals.esp.BoundingBox", defaults.visuals.esp.BoundingBox);
	visuals.esp.Skeleton    = GetBool(kv, "visuals.esp.Skeleton", defaults.visuals.esp.Skeleton);
	visuals.esp.HealthBar   = GetBool(kv, "visuals.esp.HealthBar", defaults.visuals.esp.HealthBar);
	visuals.esp.Name        = GetBool(kv, "visuals.esp.Name", defaults.visuals.esp.Name);
	visuals.esp.Dormant     = GetBool(kv, "visuals.esp.Dormant", defaults.visuals.esp.Dormant);
	visuals.esp.WeaponESP   = GetBool(kv, "visuals.esp.WeaponESP", defaults.visuals.esp.WeaponESP);
	visuals.esp.boxType     = GetInt(kv, "visuals.esp.BoxType", defaults.visuals.esp.boxType);
	visuals.esp.color       = GetColor(kv, "visuals.esp.Color", defaults.visuals.esp.color);
	visuals.esp.BoxColor      = GetColor(kv, "visuals.esp.BoxColor", defaults.visuals.esp.BoxColor);
	visuals.esp.SkeletonColor = GetColor(kv, "visuals.esp.SkeletonColor", defaults.visuals.esp.SkeletonColor);
	visuals.esp.NameColor     = GetColor(kv, "visuals.esp.NameColor", defaults.visuals.esp.NameColor);
	visuals.esp.SnapLineColor = GetColor(kv, "visuals.esp.SnapLineColor", defaults.visuals.esp.SnapLineColor);
	visuals.esp.WeaponColor   = GetColor(kv, "visuals.esp.WeaponColor", defaults.visuals.esp.WeaponColor);

	// Visuals Misc
	visuals.misc.AspectRatio = GetFloat(kv, "visuals.misc.AspectRatio", defaults.visuals.misc.AspectRatio);
	visuals.misc.ThirdPerson = GetBool(kv, "visuals.misc.ThirdPerson", defaults.visuals.misc.ThirdPerson);
	{
		unsigned int tpKey = (unsigned int)GetInt(kv, "visuals.misc.TPKey", defaults.visuals.misc.ThirdPersonKey.virtualKey);
		visuals.misc.ThirdPersonKey = Hotkey(tpKey);
		visuals.misc.ThirdPersonKey.mode = static_cast<HotkeyMode>(GetInt(kv, "visuals.misc.TPKeyMode", static_cast<int>(defaults.visuals.misc.ThirdPersonKey.mode)));
	}
	visuals.misc.TPDistance  = GetFloat(kv, "visuals.misc.TPDistance", defaults.visuals.misc.TPDistance);
	visuals.misc.camFOV      = GetFloat(kv, "visuals.misc.CamFOV", defaults.visuals.misc.camFOV);
	visuals.misc.SteadyCam   = GetBool(kv, "visuals.misc.SteadyCam", defaults.visuals.misc.SteadyCam);
	visuals.misc.NoZoom      = GetBool(kv, "visuals.misc.NoZoom", defaults.visuals.misc.NoZoom);
	visuals.misc.NightMode   = GetBool(kv, "visuals.misc.NightMode", defaults.visuals.misc.NightMode);
	visuals.misc.NightModeBrightness = GetFloat(kv, "visuals.misc.NightModeBrightness", defaults.visuals.misc.NightModeBrightness);

	// Visuals ViewModel
	visuals.viewmodel.ViewModelFOV = GetFloat(kv, "visuals.viewmodel.ViewModelFOV", defaults.visuals.viewmodel.ViewModelFOV);
	visuals.viewmodel.AlwaysDraw   = GetBool(kv, "visuals.viewmodel.AlwaysDraw", defaults.visuals.viewmodel.AlwaysDraw);

	// Visuals Glow
	visuals.glow.Enabled       = GetBool(kv, "visuals.glow.Enabled", defaults.visuals.glow.Enabled);
	visuals.glow.Friendly      = GetBool(kv, "visuals.glow.Friendly", defaults.visuals.glow.Friendly);
	visuals.glow.LocalPlayer   = GetBool(kv, "visuals.glow.LocalPlayer", defaults.visuals.glow.LocalPlayer);
	visuals.glow.SyncWithChams = GetBool(kv, "visuals.glow.SyncWithChams", defaults.visuals.glow.SyncWithChams);
	visuals.glow.Intensity     = GetFloat(kv, "visuals.glow.Intensity", defaults.visuals.glow.Intensity);
	visuals.glow.Style         = GetInt(kv, "visuals.glow.Style", defaults.visuals.glow.Style);
	visuals.glow.EnemyColor    = GetColor(kv, "visuals.glow.EnemyColor", defaults.visuals.glow.EnemyColor);
	visuals.glow.FriendlyColor = GetColor(kv, "visuals.glow.FriendlyColor", defaults.visuals.glow.FriendlyColor);
	visuals.glow.LocalColor    = GetColor(kv, "visuals.glow.LocalColor", defaults.visuals.glow.LocalColor);

	// Visuals Hitmarker
	visuals.hitmarker.Enabled      = GetBool(kv, "visuals.hitmarker.Enabled", defaults.visuals.hitmarker.Enabled);
	visuals.hitmarker.ShowDamage   = GetBool(kv, "visuals.hitmarker.ShowDamage", defaults.visuals.hitmarker.ShowDamage);
	visuals.hitmarker.Sound        = GetBool(kv, "visuals.hitmarker.Sound", defaults.visuals.hitmarker.Sound);
	visuals.hitmarker.Size         = GetFloat(kv, "visuals.hitmarker.Size", defaults.visuals.hitmarker.Size);
	visuals.hitmarker.Gap          = GetFloat(kv, "visuals.hitmarker.Gap", defaults.visuals.hitmarker.Gap);
	visuals.hitmarker.Thickness    = GetFloat(kv, "visuals.hitmarker.Thickness", defaults.visuals.hitmarker.Thickness);
	visuals.hitmarker.Duration     = GetInt(kv, "visuals.hitmarker.Duration", defaults.visuals.hitmarker.Duration);
	visuals.hitmarker.Color        = GetColor(kv, "visuals.hitmarker.Color", defaults.visuals.hitmarker.Color);
	visuals.hitmarker.HeadshotColor = GetColor(kv, "visuals.hitmarker.HeadshotColor", defaults.visuals.hitmarker.HeadshotColor);
	visuals.hitmarker.KillColor    = GetColor(kv, "visuals.hitmarker.KillColor", defaults.visuals.hitmarker.KillColor);

	// Visuals Chams
	visuals.chams.Enabled            = GetBool(kv, "visuals.chams.Enabled", defaults.visuals.chams.Enabled);
	visuals.chams.Teammates          = GetBool(kv, "visuals.chams.Teammates", defaults.visuals.chams.Teammates);
	visuals.chams.LocalPlayer        = GetBool(kv, "visuals.chams.LocalPlayer", defaults.visuals.chams.LocalPlayer);
	visuals.chams.ThroughWalls       = GetBool(kv, "visuals.chams.ThroughWalls", defaults.visuals.chams.ThroughWalls);
	visuals.chams.Style              = GetInt(kv, "visuals.chams.Style", defaults.visuals.chams.Style);
	visuals.chams.VisibleAlpha       = GetFloat(kv, "visuals.chams.VisibleAlpha", defaults.visuals.chams.VisibleAlpha);
	visuals.chams.InvisibleAlpha     = GetFloat(kv, "visuals.chams.InvisibleAlpha", defaults.visuals.chams.InvisibleAlpha);
	visuals.chams.EnemyVisibleColor  = GetColor(kv, "visuals.chams.EnemyVisibleColor", defaults.visuals.chams.EnemyVisibleColor);
	visuals.chams.EnemyInvisibleColor = GetColor(kv, "visuals.chams.EnemyInvisibleColor", defaults.visuals.chams.EnemyInvisibleColor);
	visuals.chams.FriendlyVisibleColor = GetColor(kv, "visuals.chams.FriendlyVisibleColor", defaults.visuals.chams.FriendlyVisibleColor);
	visuals.chams.LocalVisibleColor  = GetColor(kv, "visuals.chams.LocalVisibleColor", defaults.visuals.chams.LocalVisibleColor);

	// Visuals Skin Changer
	visuals.skinChanger.Enabled      = GetBool(kv, "visuals.skinChanger.Enabled", defaults.visuals.skinChanger.Enabled);
	visuals.skinChanger.KnifeModel   = GetInt(kv, "visuals.skinChanger.KnifeModel", defaults.visuals.skinChanger.KnifeModel);
	visuals.skinChanger.SkinPaintKit = GetInt(kv, "visuals.skinChanger.SkinPaintKit", defaults.visuals.skinChanger.SkinPaintKit);
	visuals.skinChanger.SkinSeed    = GetInt(kv, "visuals.skinChanger.SkinSeed", defaults.visuals.skinChanger.SkinSeed);
	visuals.skinChanger.SkinWear    = GetFloat(kv, "visuals.skinChanger.SkinWear", defaults.visuals.skinChanger.SkinWear);
	visuals.skinChanger.StatTrak    = GetInt(kv, "visuals.skinChanger.StatTrak", defaults.visuals.skinChanger.StatTrak);

	// Visuals Crosshair
	visuals.crosshair.Enabled         = GetBool(kv, "visuals.crosshair.Enabled", defaults.visuals.crosshair.Enabled);
	visuals.crosshair.Style           = GetInt(kv, "visuals.crosshair.Style", defaults.visuals.crosshair.Style);
	visuals.crosshair.Size            = GetFloat(kv, "visuals.crosshair.Size", defaults.visuals.crosshair.Size);
	visuals.crosshair.Gap             = GetFloat(kv, "visuals.crosshair.Gap", defaults.visuals.crosshair.Gap);
	visuals.crosshair.Thickness       = GetFloat(kv, "visuals.crosshair.Thickness", defaults.visuals.crosshair.Thickness);
	visuals.crosshair.Outline         = GetBool(kv, "visuals.crosshair.Outline", defaults.visuals.crosshair.Outline);
	visuals.crosshair.Color           = GetColor(kv, "visuals.crosshair.Color", defaults.visuals.crosshair.Color);
	visuals.crosshair.RecoilCrosshair = GetBool(kv, "visuals.crosshair.RecoilCrosshair", defaults.visuals.crosshair.RecoilCrosshair);
	visuals.crosshair.RecoilColor     = GetColor(kv, "visuals.crosshair.RecoilColor", defaults.visuals.crosshair.RecoilColor);
	visuals.crosshair.SniperCrosshair = GetBool(kv, "visuals.crosshair.SniperCrosshair", defaults.visuals.crosshair.SniperCrosshair);

	// Misc
	misc.RadarHack = GetBool(kv, "misc.RadarHack", defaults.misc.RadarHack);
	misc.AntiFlash = GetBool(kv, "misc.AntiFlash", defaults.misc.AntiFlash);
	misc.FlashMaxAlpha = GetFloat(kv, "misc.FlashMaxAlpha", defaults.misc.FlashMaxAlpha);
	misc.SpectatorList = GetBool(kv, "misc.SpectatorList", defaults.misc.SpectatorList);
	misc.KeybindList = GetBool(kv, "misc.KeybindList", defaults.misc.KeybindList);

	// Misc Movement
	misc.movement.BunnyHop = GetBool(kv, "misc.movement.BunnyHop", defaults.misc.movement.BunnyHop);
	misc.movement.AirDuck  = GetBool(kv, "misc.movement.AirDuck", defaults.misc.movement.AirDuck);
	misc.movement.Strafe   = GetBool(kv, "misc.movement.Strafe", defaults.misc.movement.Strafe);
	misc.movement.AutoStop = GetBool(kv, "misc.movement.AutoStop", defaults.misc.movement.AutoStop);
	misc.movement.AutoStopMode = GetInt(kv, "misc.movement.AutoStopMode", defaults.misc.movement.AutoStopMode);
	misc.movement.AutoStopSpeed = GetFloat(kv, "misc.movement.AutoStopSpeed", defaults.misc.movement.AutoStopSpeed);

	// Misc Exploits
	misc.exploits.InfDuck      = GetBool(kv, "misc.exploits.InfDuck", defaults.misc.exploits.InfDuck);
	misc.exploits.FakeLag      = GetBool(kv, "misc.exploits.FakeLag", defaults.misc.exploits.FakeLag);
	misc.exploits.FakeLagAmount = GetInt(kv, "misc.exploits.FakeLagAmount", defaults.misc.exploits.FakeLagAmount);
	misc.exploits.FakeLagVis    = GetBool(kv, "misc.exploits.FakeLagVis", defaults.misc.exploits.FakeLagVis);

	// Settings
	settings.StreamProof  = GetBool(kv, "settings.StreamProof", defaults.settings.StreamProof);
	settings.ShowDebug    = GetBool(kv, "settings.ShowDebug", defaults.settings.ShowDebug);
	settings.AnimSpeed    = GetFloat(kv, "settings.AnimSpeed", defaults.settings.AnimSpeed);
	settings.ToggleStyle  = GetBool(kv, "settings.ToggleStyle", defaults.settings.ToggleStyle);

	// Settings MouseTracer
	settings.mouseTracer.Enabled       = GetBool(kv, "settings.mouseTracer.Enabled", defaults.settings.mouseTracer.Enabled);
	settings.mouseTracer.TrailLength   = GetInt(kv, "settings.mouseTracer.TrailLength", defaults.settings.mouseTracer.TrailLength);
	settings.mouseTracer.TrailThickness = GetFloat(kv, "settings.mouseTracer.TrailThickness", defaults.settings.mouseTracer.TrailThickness);
	settings.mouseTracer.AlwaysOn      = GetBool(kv, "settings.mouseTracer.AlwaysOn", defaults.settings.mouseTracer.AlwaysOn);
	settings.mouseTracer.Color         = GetColor(kv, "settings.mouseTracer.Color", defaults.settings.mouseTracer.Color);
	settings.mouseTracer.SecondColor   = GetColor(kv, "settings.mouseTracer.SecondColor", defaults.settings.mouseTracer.SecondColor);

	return true;
}

void Config::Reset()
{
	*this = Config();
}
