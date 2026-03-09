#include "config.h"
#include "lib/utils/utils.h"

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

// Simple key=value writer. One section per category, flat within.
bool Config::Save(const std::string& name) const
{
	std::string path = GetConfigDir() + "\\" + name + ".cfg";
	std::ofstream f(path);
	if (!f.is_open())
		return false;

	f << "[aimbot]\n";
	f << "Key=" << aimbot.Key.virtualKey << "\n";
	f << "Enabled=" << aimbot.Enabled << "\n";
	f << "Silent=" << aimbot.Silent << "\n";
	f << "FOV=" << aimbot.FOV << "\n";
	f << "Smooth=" << aimbot.Smooth << "\n";
	f << "MaxPlayersInFov=" << aimbot.MaxPlayersInFov << "\n";
	f << "DrawFov=" << aimbot.DrawFov << "\n";
	f << "FriendlyFire=" << aimbot.FriendlyFire << "\n";

	f << "\n[visuals]\n";
	f << "Enabled=" << visuals.Enabled << "\n";
	f << "Friendly=" << visuals.Friendly << "\n";

	f << "\n[visuals.esp]\n";
	f << "Enabled=" << visuals.esp.Enabled << "\n";
	f << "Lines=" << visuals.esp.Lines << "\n";
	f << "BoundingBox=" << visuals.esp.BoudningBox << "\n";
	f << "Skeleton=" << visuals.esp.Skeleton << "\n";
	f << "HealthBar=" << visuals.esp.HealthBar << "\n";
	f << "Name=" << visuals.esp.Name << "\n";
	f << "Dormant=" << visuals.esp.Dormant << "\n";
	f << "BoxType=" << visuals.esp.boxType << "\n";
	f << "Color=" << visuals.esp.color.r << "," << visuals.esp.color.g << ","
	  << visuals.esp.color.b << "," << visuals.esp.color.a << "\n";

	f << "\n[visuals.misc]\n";
	f << "AspectRatio=" << visuals.misc.AspectRatio << "\n";
	f << "ThirdPerson=" << visuals.misc.ThirdPerson << "\n";
	f << "TPDistance=" << visuals.misc.TPDistance << "\n";
	f << "CamFOV=" << visuals.misc.camFOV << "\n";
	f << "SteadyCam=" << visuals.misc.SteadyCam << "\n";
	f << "NoZoom=" << visuals.misc.noZoon << "\n";

	f << "\n[visuals.viewmodel]\n";
	f << "ViewModelFOV=" << visuals.viewmodel.ViewModelFOV << "\n";
	f << "AlwaysDraw=" << visuals.viewmodel.AlwaysDraw << "\n";

	f << "\n[misc.movement]\n";
	f << "BunnyHop=" << misc.movement.BunnyHop << "\n";
	f << "AirDuck=" << misc.movement.AirDuck << "\n";
	f << "Strafe=" << misc.movement.Strafe << "\n";

	f << "\n[misc.exploits]\n";
	f << "InfDuck=" << misc.exploits.InfDuck << "\n";

	f << "\n[settings]\n";
	f << "StreamProof=" << settings.StreamProof << "\n";
	f << "ShowDebug=" << settings.ShowDebug << "\n";
	f << "AnimSpeed=" << settings.AnimSpeed << "\n";

	f << "\n[settings.mouseTracer]\n";
	f << "Enabled=" << settings.mouseTracer.Enabled << "\n";
	f << "TrailLength=" << settings.mouseTracer.TrailLength << "\n";
	f << "TrailThickness=" << settings.mouseTracer.TrailThickness << "\n";
	f << "AlwaysOn=" << settings.mouseTracer.AlwaysOn << "\n";
	f << "Color=" << settings.mouseTracer.Color.r << "," << settings.mouseTracer.Color.g << ","
	  << settings.mouseTracer.Color.b << "," << settings.mouseTracer.Color.a << "\n";
	f << "SecondColor=" << settings.mouseTracer.SecondColor.r << "," << settings.mouseTracer.SecondColor.g << ","
	  << settings.mouseTracer.SecondColor.b << "," << settings.mouseTracer.SecondColor.a << "\n";

	return f.good();
}

// Parse helpers
namespace {
	std::unordered_map<std::string, std::string> ParseINI(const std::string& path)
	{
		std::unordered_map<std::string, std::string> kv;
		std::ifstream f(path);
		if (!f.is_open()) return kv;

		std::string line, section;
		while (std::getline(f, line)) {
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

	// Visuals
	visuals.Enabled  = GetBool(kv, "visuals.Enabled", defaults.visuals.Enabled);
	visuals.Friendly = GetBool(kv, "visuals.Friendly", defaults.visuals.Friendly);

	// Visuals ESP
	visuals.esp.Enabled     = GetBool(kv, "visuals.esp.Enabled", defaults.visuals.esp.Enabled);
	visuals.esp.Lines       = GetBool(kv, "visuals.esp.Lines", defaults.visuals.esp.Lines);
	visuals.esp.BoudningBox = GetBool(kv, "visuals.esp.BoundingBox", defaults.visuals.esp.BoudningBox);
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
	visuals.misc.noZoon      = GetBool(kv, "visuals.misc.NoZoom", defaults.visuals.misc.noZoon);

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
