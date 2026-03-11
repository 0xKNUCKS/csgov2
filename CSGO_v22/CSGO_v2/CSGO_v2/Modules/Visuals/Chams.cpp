#include "Chams.h"
#include "SDK/Globals/Globals.h"
#include "SDK/Entity/localplayer.h"
#include "SDK/Classes/MaterialSystem.h"
#include "SDK/Classes/KeyValues.h"
#include "lib/Configs/config.h"
#include "lib/Error/Log.h"
#include "lib/Error/CrashLog.h"
#include <cstring>
#include <crtdbg.h>
#include <rtcapi.h>

// Suppress MSVC debug runtime dialogs (ESP check, assertions) during material creation
static int __cdecl SilentRtcHandler(int, const char*, int, const char*, const char*, ...) { return 0; }
static void __cdecl SilentInvalidParam(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t) {}

static int s_oldCrtMode;
static _RTC_error_fn s_oldRtcFn;
static _invalid_parameter_handler s_oldHandler;

static void SuppressCrtDialogs()
{
	s_oldCrtMode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	s_oldRtcFn = _RTC_SetErrorFunc(SilentRtcHandler);
	s_oldHandler = _set_invalid_parameter_handler(SilentInvalidParam);
}

static void RestoreCrtDialogs()
{
	_CrtSetReportMode(_CRT_ERROR, s_oldCrtMode);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_RTC_SetErrorFunc(s_oldRtcFn);
	_set_invalid_parameter_handler(s_oldHandler);
}

static constexpr int MAT_COUNT = 8;
static IMaterial* materials[MAT_COUNT] = {};
static bool initialized = false;

static const CfgColor* pendingVisCol = nullptr;
static IMaterial* pendingMat = nullptr;

// Helper: create a material using KeyValuesFromString + setString (Osiris pattern)
struct MatProp { const char* key; const char* val; };

static IMaterial* CreateMat(const char* name, const char* shader, const MatProp* props, int propCount)
{
	auto* kv = KeyValues::FromString(shader);
	if (!kv) {
		Log::Err("Chams", "FromString failed for '{}'", name);
		return nullptr;
	}

	for (int i = 0; i < propCount; i++)
		kv->SetString(props[i].key, props[i].val);

	auto* mat = globals::g_interfaces.MaterialSystem->CreateMaterial(name, kv);
	if (mat) {
		mat->IncrementReferenceCount();
		Log::Info("Chams", "Created '{}' at {:#x}", name, (uintptr_t)mat);
	}
	return mat;
}

static IMaterial* CreateFlat()
{
	MatProp p[] = {
		{"$basetexture", "vgui/white_additive"},
		{"$nofog", "1"}, {"$model", "1"}, {"$nocull", "0"},
		{"$selfillum", "1"}, {"$halflambert", "1"}, {"$flat", "1"},
	};
	return CreateMat("csgo_v2_flat", "UnlitGeneric", p, _countof(p));
}

static IMaterial* CreateShaded()
{
	MatProp p[] = {
		{"$basetexture", "vgui/white_additive"},
		{"$nofog", "1"}, {"$model", "1"}, {"$nocull", "0"},
		{"$halflambert", "1"},
	};
	return CreateMat("csgo_v2_shaded", "VertexLitGeneric", p, _countof(p));
}

static IMaterial* CreateChrome()
{
	MatProp p[] = {
		{"$basetexture", "vgui/white_additive"},
		{"$nofog", "1"}, {"$model", "1"},
		{"$envmap", "env_cubemap"},
		{"$envmapcontrast", "1"},
		{"$halflambert", "1"},
	};
	return CreateMat("csgo_v2_chrome", "VertexLitGeneric", p, _countof(p));
}

static IMaterial* CreateGlow()
{
	MatProp p[] = {
		{"$basetexture", "vgui/white_additive"},
		{"$nofog", "1"}, {"$model", "1"},
		{"$additive", "1"},
		{"$envmap", "models/effects/cube_white"},
		{"$envmapfresnel", "1"},
		{"$alpha", ".8"},
	};
	return CreateMat("csgo_v2_glow", "VertexLitGeneric", p, _countof(p));
}

static IMaterial* CreatePearlescent()
{
	MatProp p[] = {
		{"$basetexture", "vgui/white_additive"},
		{"$nofog", "1"}, {"$model", "1"},
		{"$ambientonly", "1"},
		{"$phong", "1"},
		{"$pearlescent", "3"},
		{"$basemapalphaphongmask", "1"},
	};
	return CreateMat("csgo_v2_pearl", "VertexLitGeneric", p, _countof(p));
}

static IMaterial* CreateGold()
{
	MatProp p[] = {
		{"$basetexture", "white"},
		{"$bumpmap", "effects/flat_normal"},
		{"$nofog", "1"}, {"$model", "1"},
		{"$envmap", "editor/cube_vertigo"},
		{"$envmapfresnel", ".6"},
		{"$phong", "1"},
		{"$phongboost", "6"},
		{"$phongexponent", "128"},
		{"$phongdisablehalflambert", "1"},
		{"$color2", "[.18 .15 .06]"},
		{"$envmaptint", "[.6 .5 .2]"},
		{"$phongfresnelranges", "[.7 .8 1]"},
		{"$phongtint", "[.6 .5 .2]"},
	};
	return CreateMat("csgo_v2_gold", "VertexLitGeneric", p, _countof(p));
}

static IMaterial* CreateCrystal()
{
	MatProp p[] = {
		{"$basetexture", "black"},
		{"$bumpmap", "effects/flat_normal"},
		{"$nofog", "1"}, {"$model", "1"},
		{"$translucent", "1"},
		{"$envmap", "models/effects/crystal_cube_vertigo_hdr"},
		{"$envmapfresnel", "0"},
		{"$phong", "1"},
		{"$phongexponent", "16"},
		{"$phongboost", "2"},
		{"$phongtint", "[.2 .35 .6]"},
	};
	return CreateMat("csgo_v2_crystal", "VertexLitGeneric", p, _countof(p));
}

static IMaterial* CreateObsidian()
{
	MatProp p[] = {
		{"$basetexture", "vgui/white_additive"},
		{"$nofog", "1"}, {"$model", "1"},
		{"$envmap", "editor/cube_vertigo"},
		{"$envmapcontrast", "1"},
		{"$envmapfresnel", ".6"},
		{"$phong", "1"},
		{"$phongboost", "2"},
		{"$phongexponent", "8"},
		{"$phongfresnelranges", "[.7 .8 1]"},
	};
	return CreateMat("csgo_v2_obsidian", "VertexLitGeneric", p, _countof(p));
}

using MatCreator = IMaterial* (*)();
static const MatCreator creators[MAT_COUNT] = {
	CreateFlat, CreateShaded, CreateChrome, CreateGlow,
	CreatePearlescent, CreateGold, CreateCrystal, CreateObsidian,
};

bool chams::Init()
{
	if (initialized) return true;

	if (!globals::g_interfaces.MaterialSystem || !globals::g_interfaces.ModelRender) {
		CrashLog::Write("[Chams] MaterialSystem or ModelRender is null");
		return false;
	}

	int created = 0;
	SuppressCrtDialogs();
	for (int i = 0; i < MAT_COUNT; i++) {
		__try {
			materials[i] = creators[i]();
			if (materials[i]) created++;
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			Log::Err("Chams", "Crashed creating material index {}", i);
			materials[i] = nullptr;
		}
	}
	RestoreCrtDialogs();

	if (created == 0) {
		CrashLog::Write("[Chams] No materials created");
		return false;
	}

	initialized = true;
	Log::Info("Chams", "Initialized ({}/{} materials)", created, MAT_COUNT);
	CrashLog::Write("[Chams] Initialized");
	return true;
}

void chams::Shutdown()
{
	for (auto& m : materials) m = nullptr;
	initialized = false;
}

static IMaterial* GetMaterial(int style)
{
	if (style >= 0 && style < MAT_COUNT && materials[style])
		return materials[style];
	return materials[0];
}

chams::Result chams::OnDrawModel(const ModelRenderInfo_t& info)
{
	pendingVisCol = nullptr;
	pendingMat = nullptr;

	if (!initialized || !cfg.visuals.chams.Enabled)
		return Result::None;

	if (!globals::g_interfaces.Engine->IsInGame())
		return Result::None;

	if (!info.pModel)
		return Result::None;

	const char* modelName = info.pModel->name;
	if (!modelName || !strstr(modelName, "models/player"))
		return Result::None;

	auto* ent = globals::g_interfaces.ClientEntity->GetClientEntity(info.entity_index);
	if (!ent || !ent->isValidState() || ent->isDormant())
		return Result::None;

	gEntity* lp = LocalPlayer.Get();
	if (!lp) return Result::None;

	bool isTeammate = ent->isTeammate();
	bool isLocal = (info.entity_index == globals::g_interfaces.Engine->GetLocalPlayerIdx());

	if (isTeammate && !isLocal && !cfg.visuals.chams.Teammates)
		return Result::None;
	if (isLocal && !cfg.visuals.chams.LocalPlayer)
		return Result::None;

	const CfgColor* visCol;
	const CfgColor* invisCol;

	if (isLocal) {
		visCol = &cfg.visuals.chams.LocalVisibleColor;
		invisCol = visCol;
	} else if (isTeammate) {
		visCol = &cfg.visuals.chams.FriendlyVisibleColor;
		invisCol = visCol;
	} else {
		visCol = &cfg.visuals.chams.EnemyVisibleColor;
		invisCol = &cfg.visuals.chams.EnemyInvisibleColor;
	}

	IMaterial* mat = GetMaterial(cfg.visuals.chams.Style);
	if (!mat) return Result::None;

	if (cfg.visuals.chams.ThroughWalls && !isTeammate) {
		mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
		mat->ColorModulate(invisCol->r, invisCol->g, invisCol->b);
		mat->AlphaModulate(invisCol->a);
		globals::g_interfaces.ModelRender->ForcedMaterialOverride(mat);

		pendingVisCol = visCol;
		pendingMat = mat;
		return Result::ThroughWalls;
	}

	mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
	mat->ColorModulate(visCol->r, visCol->g, visCol->b);
	mat->AlphaModulate(visCol->a);
	globals::g_interfaces.ModelRender->ForcedMaterialOverride(mat);
	return Result::VisibleOnly;
}

void chams::SetupVisiblePass()
{
	if (!pendingMat || !pendingVisCol) return;
	pendingMat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
	pendingMat->ColorModulate(pendingVisCol->r, pendingVisCol->g, pendingVisCol->b);
	pendingMat->AlphaModulate(pendingVisCol->a);
	globals::g_interfaces.ModelRender->ForcedMaterialOverride(pendingMat);
}

void chams::ClearOverride()
{
	globals::g_interfaces.ModelRender->ForcedMaterialOverride(nullptr);
}
