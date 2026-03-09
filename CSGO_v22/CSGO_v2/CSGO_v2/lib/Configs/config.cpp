#include "config.h"
#include "lib/utils/utils.h"
#include "MD5.h"

Hotkey::Hotkey(unsigned int key)
	: virtualKey(key)
	, label(utils::VirtualKeyToString(key))
{
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
	body << "Enabled=" << aimbot.Enabled << "\n";
	body << "Silent=" << aimbot.Silent << "\n";
	body << "FOV=" << aimbot.FOV << "\n";
	body << "Smooth=" << aimbot.Smooth << "\n";
	body << "SmoothX=" << aimbot.SmoothX << "\n";
	body << "SmoothY=" << aimbot.SmoothY << "\n";
	body << "MaxPlayersInFov=" << aimbot.MaxPlayersInFov << "\n";
	body << "DrawFov=" << aimbot.DrawFov << "\n";
	body << "FriendlyFire=" << aimbot.FriendlyFire << "\n";
	body << "VisibilityCheck=" << aimbot.VisibilityCheck << "\n";
	body << "AimBone=" << aimbot.AimBone << "\n";
	body << "RCS=" << aimbot.RCS << "\n";
	body << "StandaloneRCS=" << aimbot.StandaloneRCS << "\n";
	body << "RCSAmountX=" << aimbot.RCSAmountX << "\n";
	body << "RCSAmountY=" << aimbot.RCSAmountY << "\n";
	body << "RCSStartBullet=" << aimbot.RCSStartBullet << "\n";
	body << "RCSSmooth=" << aimbot.RCSSmooth << "\n";
	body << "AutoShoot=" << aimbot.AutoShoot << "\n";
	body << "AutoShootFov=" << aimbot.AutoShootFov << "\n";

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
	body << "BoxType=" << visuals.esp.boxType << "\n";
	body << "Color=" << visuals.esp.color.r << "," << visuals.esp.color.g << ","
	  << visuals.esp.color.b << "," << visuals.esp.color.a << "\n";

	body << "\n[visuals.misc]\n";
	body << "AspectRatio=" << visuals.misc.AspectRatio << "\n";
	body << "ThirdPerson=" << visuals.misc.ThirdPerson << "\n";
	body << "TPDistance=" << visuals.misc.TPDistance << "\n";
	body << "CamFOV=" << visuals.misc.camFOV << "\n";
	body << "SteadyCam=" << visuals.misc.SteadyCam << "\n";
	body << "NoZoom=" << visuals.misc.NoZoom << "\n";

	body << "\n[visuals.viewmodel]\n";
	body << "ViewModelFOV=" << visuals.viewmodel.ViewModelFOV << "\n";
	body << "AlwaysDraw=" << visuals.viewmodel.AlwaysDraw << "\n";

	body << "\n[misc.movement]\n";
	body << "BunnyHop=" << misc.movement.BunnyHop << "\n";
	body << "AirDuck=" << misc.movement.AirDuck << "\n";
	body << "Strafe=" << misc.movement.Strafe << "\n";

	body << "\n[misc.exploits]\n";
	body << "InfDuck=" << misc.exploits.InfDuck << "\n";

	body << "\n[settings]\n";
	body << "StreamProof=" << settings.StreamProof << "\n";
	body << "ShowDebug=" << settings.ShowDebug << "\n";
	body << "AnimSpeed=" << settings.AnimSpeed << "\n";

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
	aimbot.RCSAmountX    = GetFloat(kv, "aimbot.RCSAmountX", defaults.aimbot.RCSAmountX);
	aimbot.RCSAmountY    = GetFloat(kv, "aimbot.RCSAmountY", defaults.aimbot.RCSAmountY);
	aimbot.RCSStartBullet = GetInt(kv, "aimbot.RCSStartBullet", defaults.aimbot.RCSStartBullet);
	aimbot.RCSSmooth     = GetFloat(kv, "aimbot.RCSSmooth", defaults.aimbot.RCSSmooth);
	aimbot.AutoShoot     = GetBool(kv, "aimbot.AutoShoot", defaults.aimbot.AutoShoot);
	aimbot.AutoShootFov  = GetFloat(kv, "aimbot.AutoShootFov", defaults.aimbot.AutoShootFov);

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
	visuals.esp.boxType     = GetInt(kv, "visuals.esp.BoxType", defaults.visuals.esp.boxType);
	visuals.esp.color       = GetColor(kv, "visuals.esp.Color", defaults.visuals.esp.color);

	// Visuals Misc
	visuals.misc.AspectRatio = GetFloat(kv, "visuals.misc.AspectRatio", defaults.visuals.misc.AspectRatio);
	visuals.misc.ThirdPerson = GetBool(kv, "visuals.misc.ThirdPerson", defaults.visuals.misc.ThirdPerson);
	visuals.misc.TPDistance  = GetFloat(kv, "visuals.misc.TPDistance", defaults.visuals.misc.TPDistance);
	visuals.misc.camFOV      = GetFloat(kv, "visuals.misc.CamFOV", defaults.visuals.misc.camFOV);
	visuals.misc.SteadyCam   = GetBool(kv, "visuals.misc.SteadyCam", defaults.visuals.misc.SteadyCam);
	visuals.misc.NoZoom      = GetBool(kv, "visuals.misc.NoZoom", defaults.visuals.misc.NoZoom);

	// Visuals ViewModel
	visuals.viewmodel.ViewModelFOV = GetFloat(kv, "visuals.viewmodel.ViewModelFOV", defaults.visuals.viewmodel.ViewModelFOV);
	visuals.viewmodel.AlwaysDraw   = GetBool(kv, "visuals.viewmodel.AlwaysDraw", defaults.visuals.viewmodel.AlwaysDraw);

	// Misc Movement
	misc.movement.BunnyHop = GetBool(kv, "misc.movement.BunnyHop", defaults.misc.movement.BunnyHop);
	misc.movement.AirDuck  = GetBool(kv, "misc.movement.AirDuck", defaults.misc.movement.AirDuck);
	misc.movement.Strafe   = GetBool(kv, "misc.movement.Strafe", defaults.misc.movement.Strafe);

	// Misc Exploits
	misc.exploits.InfDuck = GetBool(kv, "misc.exploits.InfDuck", defaults.misc.exploits.InfDuck);

	// Settings
	settings.StreamProof = GetBool(kv, "settings.StreamProof", defaults.settings.StreamProof);
	settings.ShowDebug   = GetBool(kv, "settings.ShowDebug", defaults.settings.ShowDebug);
	settings.AnimSpeed   = GetFloat(kv, "settings.AnimSpeed", defaults.settings.AnimSpeed);

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
