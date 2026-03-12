#include "SkinChanger.h"
#include "SDK/Entity/entity.h"
#include "SDK/Entity/localplayer.h"
#include "SDK/Globals/Globals.h"
#include "SDK/Classes/PlayerInfo.h"
#include "lib/Hooks/hook.h"
#include "lib/Configs/config.h"
#include "lib/Notify/Notify.h"
#include "lib/Error/Log.h"
#include <cstring>
#include <random>

static const std::vector<KnifeModel> kKnifeModels = {
	{ WEAPON_NONE, "Default", nullptr, nullptr },  // Index 0 = auto-detect stock knife by team
	{ KNIFE_BAYONET,         "Bayonet",          "models/weapons/v_knife_bayonet.mdl",          "models/weapons/w_knife_bayonet.mdl" },
	{ KNIFE_FLIP,            "Flip Knife",        "models/weapons/v_knife_flip.mdl",             "models/weapons/w_knife_flip.mdl" },
	{ KNIFE_GUT,             "Gut Knife",         "models/weapons/v_knife_gut.mdl",              "models/weapons/w_knife_gut.mdl" },
	{ KNIFE_KARAMBIT,        "Karambit",          "models/weapons/v_knife_karam.mdl",            "models/weapons/w_knife_karam.mdl" },
	{ KNIFE_M9_BAYONET,      "M9 Bayonet",        "models/weapons/v_knife_m9_bay.mdl",           "models/weapons/w_knife_m9_bay.mdl" },
	{ KNIFE_HUNTSMAN,        "Huntsman Knife",    "models/weapons/v_knife_tactical.mdl",         "models/weapons/w_knife_tactical.mdl" },
	{ KNIFE_BUTTERFLY,       "Butterfly Knife",   "models/weapons/v_knife_butterfly.mdl",        "models/weapons/w_knife_butterfly.mdl" },
	{ KNIFE_FALCHION,        "Falchion Knife",    "models/weapons/v_knife_falchion_advanced.mdl", "models/weapons/w_knife_falchion_advanced.mdl" },
	{ KNIFE_BOWIE,           "Bowie Knife",       "models/weapons/v_knife_survival_bowie.mdl",   "models/weapons/w_knife_survival_bowie.mdl" },
	{ KNIFE_SHADOW_DAGGERS,  "Shadow Daggers",    "models/weapons/v_knife_push.mdl",             "models/weapons/w_knife_push.mdl" },
	{ KNIFE_URSUS,           "Ursus Knife",       "models/weapons/v_knife_ursus.mdl",            "models/weapons/w_knife_ursus.mdl" },
	{ KNIFE_NAVAJA,          "Navaja Knife",      "models/weapons/v_knife_gypsy_jackknife.mdl",  "models/weapons/w_knife_gypsy_jackknife.mdl" },
	{ KNIFE_STILETTO,        "Stiletto Knife",    "models/weapons/v_knife_stiletto.mdl",         "models/weapons/w_knife_stiletto.mdl" },
	{ KNIFE_TALON,           "Talon Knife",       "models/weapons/v_knife_widowmaker.mdl",       "models/weapons/w_knife_widowmaker.mdl" },
	{ KNIFE_SKELETON,        "Skeleton Knife",    "models/weapons/v_knife_skeleton.mdl",         "models/weapons/w_knife_skeleton.mdl" },
	{ KNIFE_NOMAD,           "Nomad Knife",       "models/weapons/v_knife_outdoor.mdl",          "models/weapons/w_knife_outdoor.mdl" },
	{ KNIFE_PARACORD,        "Paracord Knife",    "models/weapons/v_knife_cord.mdl",             "models/weapons/w_knife_cord.mdl" },
	{ KNIFE_SURVIVAL,        "Survival Knife",    "models/weapons/v_knife_canis.mdl",            "models/weapons/w_knife_canis.mdl" },
	{ KNIFE_CSS,             "Classic Knife",     "models/weapons/v_knife_css.mdl",              "models/weapons/w_knife_css.mdl" },
};

static const std::vector<SkinInfo> kPopularSkins = {
	{ 0,   "Default" },
	{ 12,  "Crimson Web" },
	{ 38,  "Fade" },
	{ 40,  "Night" },
	{ 42,  "Blue Steel" },
	{ 43,  "Stained" },
	{ 44,  "Case Hardened" },
	{ 59,  "Slaughter" },
	{ 98,  "Ultraviolet" },
	{ 409, "Tiger Tooth" },
	{ 413, "Marble Fade" },
	{ 414, "Rust Coat" },
	{ 415, "Sapphire" },
	{ 416, "Black Pearl" },
	{ 417, "Ruby" },
	{ 418, "Emerald" },
	{ 568, "Lore" },
	{ 569, "Autotronic" },
	{ 617, "Doppler Phase 1" },
	{ 618, "Doppler Phase 2" },
	{ 619, "Doppler Phase 3" },
	{ 620, "Doppler Phase 4" },
	{ 180, "Howl" },
	{ 383, "Asiimov" },
	{ 474, "Fire Serpent" },
	{ 597, "Bloodsport" },
	{ 711, "Hyper Beast" },
	{ 846, "Neon Rider" },
	{ 856, "Printstream" },
};

const std::vector<KnifeModel>& skinchanger::GetKnifeModels() { return kKnifeModels; }
const std::vector<SkinInfo>& skinchanger::GetPopularSkins() { return kPopularSkins; }

bool skinchanger::IsKnife(short defIndex)
{
	return defIndex == WEAPON_KNIFE_CT || defIndex == WEAPON_KNIFE_T ||
		(defIndex >= 500 && defIndex <= 525);
}

// ============================================================================
// Material cache offsets (internal C++ members, not netvars)
// These are used by every leaked cheat — stable across CS:GO builds.
//
// m_Item base = m_iItemIDHigh_absolute - m_iItemIDHigh_relative_in_m_Item
//             = 12240 - 512 = 11728 (0x2DD0)
// ============================================================================
namespace skinoffsets {
	// CUtlVector size is at offset 0x0C within the vector struct.
	// To clear a vector, we set *(int*)(vectorAddr + 0x0C) = 0.
	constexpr int kUtlVecSizeOffset = 0x0C;

	// m_CustomMaterials: CUtlVector at m_Item + 0x14
	// (from Pandora-V3/Supremacy: g_netvars.get("DT_BaseCombatWeapon", "m_Item") + 0x14)
	constexpr uintptr_t m_CustomMaterials = 11728 + 0x14;  // 11748 = 0x2DE4

	// m_VisualsDataProcessors: CUtlVector at m_Item + 0x220
	constexpr uintptr_t m_VisualsDataProcessors = 11728 + 0x220;  // 12272 = 0x2FF4

	// m_CustomMaterials2: hardcoded 0x9DC (Pandora/Supremacy)
	constexpr uintptr_t m_CustomMaterials2 = 0x9DC;

	// m_bCustomMaterialInitialized: sig scanned from client.dll
	// Pattern: C6 86 ?? ?? 00 00 ?? FF 50 04 → mov byte ptr [esi+0x3370], 1
	// Old Pandora/Supremacy value was 0x32DD — WRONG for our build!
	constexpr uintptr_t m_bCustomMaterialInitialized = 0x3370;
}

static bool g_needsRefresh = false;
static bool g_initialized = false;

// Applied settings — only updated when user presses Apply.
// ApplyToWeapon uses these, NOT cfg directly, so changes don't take effect
// until the user explicitly applies them.
static struct {
	int KnifeModel = 0;
	int SkinPaintKit = 0;
	int SkinSeed = 0;
	float SkinWear = 0.0001f;
	int StatTrak = -1;
} g_applied;

void skinchanger::ForceUpdate()
{
	// Snapshot current config values as the "applied" state
	g_applied.KnifeModel = cfg.visuals.skinChanger.KnifeModel;
	g_applied.SkinPaintKit = cfg.visuals.skinChanger.SkinPaintKit;
	g_applied.SkinSeed = cfg.visuals.skinChanger.SkinSeed;
	g_applied.SkinWear = cfg.visuals.skinChanger.SkinWear;
	g_applied.StatTrak = cfg.visuals.skinChanger.StatTrak;
	g_needsRefresh = true;
}

// ============================================================================
// Knife animation sequence remapping
// ============================================================================

enum KnifeSequence {
	SEQ_DEFAULT_DRAW = 0,
	SEQ_DEFAULT_IDLE1 = 1,
	SEQ_DEFAULT_IDLE2 = 2,
	SEQ_DEFAULT_LIGHT_MISS1 = 3,
	SEQ_DEFAULT_LIGHT_MISS2 = 4,
	SEQ_DEFAULT_HEAVY_MISS1 = 9,
	SEQ_DEFAULT_HEAVY_HIT1 = 10,
	SEQ_DEFAULT_HEAVY_BACKSTAB = 11,
	SEQ_DEFAULT_LOOKAT01 = 12,

	SEQ_BUTTERFLY_DRAW = 0,
	SEQ_BUTTERFLY_DRAW2 = 1,
	SEQ_BUTTERFLY_LOOKAT01 = 13,
	SEQ_BUTTERFLY_LOOKAT03 = 15,

	SEQ_FALCHION_IDLE1 = 1,
	SEQ_FALCHION_HEAVY_MISS1 = 8,
	SEQ_FALCHION_HEAVY_MISS1_NOFLIP = 9,
	SEQ_FALCHION_LOOKAT01 = 12,
	SEQ_FALCHION_LOOKAT02 = 13,

	SEQ_DAGGERS_IDLE1 = 1,
	SEQ_DAGGERS_LIGHT_MISS1 = 2,
	SEQ_DAGGERS_LIGHT_MISS5 = 6,
	SEQ_DAGGERS_HEAVY_MISS2 = 11,
	SEQ_DAGGERS_HEAVY_MISS1 = 12,

	SEQ_BOWIE_IDLE1 = 1,
};

static std::mt19937 g_rng(std::random_device{}());

static int RandomInt(int min, int max)
{
	return std::uniform_int_distribution<>(min, max)(g_rng);
}

static int RemapKnifeSequence(short knifeDefIndex, int sequence)
{
	switch (knifeDefIndex)
	{
	case KNIFE_BUTTERFLY:
		switch (sequence) {
		case SEQ_DEFAULT_DRAW:
			return RandomInt(SEQ_BUTTERFLY_DRAW, SEQ_BUTTERFLY_DRAW2);
		case SEQ_DEFAULT_LOOKAT01:
			return RandomInt(SEQ_BUTTERFLY_LOOKAT01, SEQ_BUTTERFLY_LOOKAT03);
		default:
			return sequence + 1;
		}

	case KNIFE_FALCHION:
		switch (sequence) {
		case SEQ_DEFAULT_DRAW:
		case SEQ_DEFAULT_IDLE1:
		case SEQ_DEFAULT_HEAVY_HIT1:
		case SEQ_DEFAULT_HEAVY_BACKSTAB:
			return sequence;
		case SEQ_DEFAULT_HEAVY_MISS1:
			return RandomInt(SEQ_FALCHION_HEAVY_MISS1, SEQ_FALCHION_HEAVY_MISS1_NOFLIP);
		case SEQ_DEFAULT_LOOKAT01:
			return RandomInt(SEQ_FALCHION_LOOKAT01, SEQ_FALCHION_LOOKAT02);
		default:
			return sequence - 1;
		}

	case KNIFE_SHADOW_DAGGERS:
		switch (sequence) {
		case SEQ_DEFAULT_DRAW:
		case SEQ_DEFAULT_IDLE1:
			return sequence;
		case SEQ_DEFAULT_IDLE2:
			return SEQ_DAGGERS_IDLE1;
		case SEQ_DEFAULT_LIGHT_MISS1:
		case SEQ_DEFAULT_LIGHT_MISS2:
			return RandomInt(SEQ_DAGGERS_LIGHT_MISS1, SEQ_DAGGERS_LIGHT_MISS5);
		case SEQ_DEFAULT_HEAVY_MISS1:
			return RandomInt(SEQ_DAGGERS_HEAVY_MISS2, SEQ_DAGGERS_HEAVY_MISS1);
		case SEQ_DEFAULT_HEAVY_HIT1:
		case SEQ_DEFAULT_HEAVY_BACKSTAB:
		case SEQ_DEFAULT_LOOKAT01:
			return sequence + 3;
		default:
			return sequence + 2;
		}

	case KNIFE_BOWIE:
		switch (sequence) {
		case SEQ_DEFAULT_DRAW:
		case SEQ_DEFAULT_IDLE1:
			return sequence;
		case SEQ_DEFAULT_IDLE2:
			return SEQ_BOWIE_IDLE1;
		default:
			return sequence - 1;
		}

	case KNIFE_URSUS:
	case KNIFE_SKELETON:
	case KNIFE_NOMAD:
	case KNIFE_PARACORD:
	case KNIFE_SURVIVAL:
		switch (sequence) {
		case SEQ_DEFAULT_DRAW:
			return RandomInt(SEQ_BUTTERFLY_DRAW, SEQ_BUTTERFLY_DRAW2);
		case SEQ_DEFAULT_LOOKAT01:
			return RandomInt(SEQ_BUTTERFLY_LOOKAT01, 14);
		default:
			return sequence + 1;
		}

	case KNIFE_STILETTO:
		if (sequence == SEQ_DEFAULT_LOOKAT01)
			return RandomInt(12, 13);
		return sequence;

	case KNIFE_TALON:
		if (sequence == SEQ_DEFAULT_LOOKAT01)
			return RandomInt(14, 15);
		return sequence;

	default:
		return sequence;
	}
}

// ============================================================================
// RecvProxy hook for m_nSequence (knife animation fix)
// ============================================================================

struct ProxyHook {
	RecvVarProxyFn original;
	void** address;
};

static ProxyHook g_seqHook{};

static void RestoreHook(ProxyHook& hook)
{
	if (hook.address && hook.original) {
		*hook.address = (void*)hook.original;
		hook.original = nullptr;
		hook.address = nullptr;
	}
}

static void __cdecl SetViewModelSequence(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	CRecvProxyData modifiedData = *pData;

	if (cfg.visuals.skinChanger.Enabled && g_applied.KnifeModel > 0)
	{
		if (LocalPlayer.Get())
		{
			__try {
				int activeHandle = *(int*)((uintptr_t)LocalPlayer.Get() + offsets::m_hActiveWeapon);
				if (activeHandle && activeHandle != -1)
				{
					auto* weapon = globals::g_interfaces.ClientEntity->GetClientEntity(activeHandle & 0xFFF);
					if (weapon)
					{
						short defIdx = *(short*)((uintptr_t)weapon + offsets::m_iItemDefinitionIndex);
						if (skinchanger::IsKnife(defIdx))
							modifiedData.m_Value.m_Int = RemapKnifeSequence(defIdx, modifiedData.m_Value.m_Int);
					}
				}
			} __except (EXCEPTION_EXECUTE_HANDLER) {}
		}
	}

	if (g_seqHook.original)
		g_seqHook.original(&modifiedData, pStruct, pOut);
}

void skinchanger::InstallHooks()
{
	ClientClass* allClasses = globals::g_interfaces.BaseClient->GetAllClasses();
	for (auto* pClass = allClasses; pClass; pClass = pClass->m_pNext)
	{
		if (strcmp(pClass->m_pNetworkName, "CBaseViewModel") != 0)
			continue;

		RecvTable* table = pClass->m_pRecvTable;
		for (int i = 0; i < table->m_nProps; i++)
		{
			RecvProp* prop = &table->m_pProps[i];
			if (!prop || strcmp(prop->m_pVarName, "m_nSequence") != 0)
				continue;

			g_seqHook.original = (RecvVarProxyFn)prop->m_ProxyFn;
			g_seqHook.address = &prop->m_ProxyFn;
			prop->m_ProxyFn = (void*)SetViewModelSequence;
			Log::Info("SkinChanger", "Installed m_nSequence proxy");
			return;
		}
	}
	Log::Err("SkinChanger", "Failed to hook m_nSequence");
}

void skinchanger::RemoveHooks()
{
	RestoreHook(g_seqHook);
	Log::Info("SkinChanger", "Removed proxy hooks");
}

// ============================================================================
// Core skin changer
// ============================================================================

static const KnifeModel* GetSelectedKnife()
{
	int idx = g_applied.KnifeModel;
	if (idx <= 0 || idx >= (int)kKnifeModels.size())
		return nullptr;  // Index 0 = Default, no custom knife
	return &kKnifeModels[idx];
}

// Get the stock knife def index and model paths based on player team
static short GetStockKnifeId(int team) {
	return (team == 3) ? WEAPON_KNIFE_CT : WEAPON_KNIFE_T;
}
static const char* GetStockViewModel(int team) {
	return (team == 3)
		? "models/weapons/v_knife_default_ct.mdl"
		: "models/weapons/v_knife_default_t.mdl";
}
static const char* GetStockWorldModel(int team) {
	return (team == 3)
		? "models/weapons/w_knife_default_ct.mdl"
		: "models/weapons/w_knife_default_t.mdl";
}

static void ApplyToWeapon(gEntity* weapon, int accountId, int team)
{
	short* pDefIndex = (short*)((uintptr_t)weapon + offsets::m_iItemDefinitionIndex);
	short defIndex = *pDefIndex;

	bool isKnife = skinchanger::IsKnife(defIndex);
	const KnifeModel* knife = GetSelectedKnife();  // nullptr = Default

	// If default knife + default skin, don't modify knives at all
	if (isKnife && !knife && g_applied.SkinPaintKit == 0)
	{
		// Reset defIndex to stock (in case we overrode it on a previous frame)
		*pDefIndex = GetStockKnifeId(team);
		*(int*)((uintptr_t)weapon + offsets::m_iEntityQuality) = 0;
		return;  // Don't force fallback, let the game show the stock knife
	}

	// Force fallback path
	*(int*)((uintptr_t)weapon + offsets::m_iItemIDHigh) = -1;
	*(int*)((uintptr_t)weapon + offsets::m_iItemIDLow) = -1;
	*(int*)((uintptr_t)weapon + offsets::m_iAccountID) = accountId;

	// Knife model override
	if (isKnife)
	{
		if (knife)
		{
			// Custom knife selected
			*pDefIndex = knife->id;
			*(int*)((uintptr_t)weapon + offsets::m_iEntityQuality) = 3;

			if (knife->model)
			{
				int modelIdx = globals::g_interfaces.ModelInfo->GetModelIndex(knife->model);
				weapon->setModelIndex(modelIdx);
				*(int*)((uintptr_t)weapon + offsets::m_nModelIndex) = modelIdx;
			}
		}
		else
		{
			// Default knife + custom skin: reset model but keep skin
			*pDefIndex = GetStockKnifeId(team);
			*(int*)((uintptr_t)weapon + offsets::m_iEntityQuality) = 0;
		}
	}

	// Fallback skin values (from applied state, not live config)
	*(int*)((uintptr_t)weapon + offsets::m_nFallbackPaintKit) = g_applied.SkinPaintKit;
	*(int*)((uintptr_t)weapon + offsets::m_nFallbackSeed) = g_applied.SkinSeed;
	*(float*)((uintptr_t)weapon + offsets::m_flFallbackWear) = g_applied.SkinWear;
	*(int*)((uintptr_t)weapon + offsets::m_nFallbackStatTrak) = g_applied.StatTrak;
	*(int*)((uintptr_t)weapon + offsets::m_OriginalOwnerXuidLow) = 0;
	*(int*)((uintptr_t)weapon + offsets::m_OriginalOwnerXuidHigh) = 0;
}

// Try to clear the engine's cached materials so new paint kit textures load.
// These offsets are from Pandora/Supremacy and may vary across CS:GO builds.
// Wrapped in its own SEH handler so a crash here doesn't block the critical
// PostDataUpdate/OnDataChanged calls below.
static void TryClearMaterialCaches(gEntity* weapon)
{
	uintptr_t wpn = (uintptr_t)weapon;

	__try {
		*(bool*)(wpn + skinoffsets::m_bCustomMaterialInitialized) = false;
		*(int*)(wpn + skinoffsets::m_CustomMaterials + skinoffsets::kUtlVecSizeOffset) = 0;
		*(int*)(wpn + skinoffsets::m_CustomMaterials2 + skinoffsets::kUtlVecSizeOffset) = 0;
		*(int*)(wpn + skinoffsets::m_VisualsDataProcessors + skinoffsets::kUtlVecSizeOffset) = 0;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

// Force the engine to re-run InitializeAttributes with our modified netvars.
// Uses multiple strategies:
// 1. PreDataUpdate(0) — Legendware approach: tells engine entity is "new"
// 2. Material cache clearing — Pandora approach (offsets may be wrong)
// 3. PostDataUpdate(0) + OnDataChanged(0) — manual InitializeAttributes trigger
static void ForceItemUpdate(gEntity* weapon)
{
	__try { weapon->preDataUpdate(0); } __except (EXCEPTION_EXECUTE_HANDLER) {}
	TryClearMaterialCaches(weapon);
	__try { weapon->postDataUpdate(0); } __except (EXCEPTION_EXECUTE_HANDLER) {}
	__try { weapon->onDataChanged(0); } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void UpdateViewModel(gEntity* localPlayer, int team)
{
	int activeHandle = *(int*)((uintptr_t)localPlayer + offsets::m_hActiveWeapon);
	if (!activeHandle || activeHandle == -1)
		return;

	gEntity* activeWeapon = globals::g_interfaces.ClientEntity->GetClientEntity(activeHandle & 0xFFF);
	if (!activeWeapon)
		return;

	short defIdx = *(short*)((uintptr_t)activeWeapon + offsets::m_iItemDefinitionIndex);
	if (!skinchanger::IsKnife(defIdx))
		return;

	const KnifeModel* knife = GetSelectedKnife();

	// Determine which model to use: custom knife or stock (auto-detect team)
	const char* vmodel = knife ? knife->model : GetStockViewModel(team);
	const char* wmodel = knife ? knife->wmodel : GetStockWorldModel(team);

	if (!vmodel)
		return;

	int vmHandle = *(int*)((uintptr_t)localPlayer + offsets::m_hViewModel);
	if (vmHandle && vmHandle != -1)
	{
		gEntity* viewModel = globals::g_interfaces.ClientEntity->GetClientEntity(vmHandle & 0xFFF);
		if (viewModel)
		{
			int modelIdx = globals::g_interfaces.ModelInfo->GetModelIndex(vmodel);
			viewModel->setModelIndex(modelIdx);
			*(int*)((uintptr_t)viewModel + offsets::m_nModelIndex) = modelIdx;
		}
	}

	if (wmodel)
	{
		int wmodelHandle = *(int*)((uintptr_t)activeWeapon + offsets::m_hWeaponWorldModel);
		if (wmodelHandle && wmodelHandle != -1)
		{
			gEntity* worldModel = globals::g_interfaces.ClientEntity->GetClientEntity(wmodelHandle & 0xFFF);
			if (worldModel)
			{
				int wmodelIdx = globals::g_interfaces.ModelInfo->GetModelIndex(wmodel);
				*(int*)((uintptr_t)worldModel + offsets::m_nModelIndex) = wmodelIdx;
			}
		}
	}
}

void skinchanger::Run()
{
	if (!cfg.visuals.skinChanger.Enabled)
		return;

	if (!globals::g_interfaces.Engine || !globals::g_interfaces.Engine->IsInGame() || !hooks::GlobalVars)
		return;

	gEntity* lp = LocalPlayer.Get();
	if (!lp)
		return;

	bool alive = *(int*)((uintptr_t)lp + offsets::deadFlag) == 0;
	if (!alive)
		return;

	// On first run, sync g_applied from saved config so skins persist across reinjections
	if (!g_initialized)
	{
		g_applied.KnifeModel = cfg.visuals.skinChanger.KnifeModel;
		g_applied.SkinPaintKit = cfg.visuals.skinChanger.SkinPaintKit;
		g_applied.SkinSeed = cfg.visuals.skinChanger.SkinSeed;
		g_applied.SkinWear = cfg.visuals.skinChanger.SkinWear;
		g_applied.StatTrak = cfg.visuals.skinChanger.StatTrak;
		g_initialized = true;
		g_needsRefresh = true;
	}

	player_info_s pinfo{};
	if (!globals::g_interfaces.Engine->getPlayerInfo(globals::g_interfaces.Engine->GetLocalPlayerIdx(), pinfo))
		return;
	int accountId = (int)pinfo.xuidLow;
	int team = *(int*)((uintptr_t)lp + offsets::m_iTeamNum);

	bool doRefresh = g_needsRefresh;
	if (doRefresh)
		g_needsRefresh = false;

	int weaponCount = 0;

	// Apply netvars to ALL weapons every frame
	for (int i = 0; i < 48; i++)
	{
		int handle = *(int*)((uintptr_t)lp + offsets::m_hMyWeapons + i * 4);
		if (!handle || handle == -1)
			continue;

		gEntity* weapon = globals::g_interfaces.ClientEntity->GetClientEntity(handle & 0xFFF);
		if (!weapon || weapon->isDormant())
			continue;

		__try {
			ApplyToWeapon(weapon, accountId, team);
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			Log::Err("SkinChanger", "CRASH in ApplyToWeapon for weapon slot {}", i);
		}

		if (doRefresh)
		{
			ForceItemUpdate(weapon);
			weaponCount++;
		}
	}

	// Update viewmodel + world model for knife (always — handles both custom and stock reset)
	__try {
		UpdateViewModel(lp, team);
	} __except (EXCEPTION_EXECUTE_HANDLER) {}
}
