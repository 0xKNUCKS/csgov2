#pragma once
#include <string>
#include <vector>

// Weapon definition IDs
enum WeaponId : short {
	WEAPON_NONE = 0,
	WEAPON_DEAGLE = 1,
	WEAPON_ELITE = 2,
	WEAPON_FIVESEVEN = 3,
	WEAPON_GLOCK = 4,
	WEAPON_AK47 = 7,
	WEAPON_AUG = 8,
	WEAPON_AWP = 9,
	WEAPON_FAMAS = 10,
	WEAPON_G3SG1 = 11,
	WEAPON_GALIL = 13,
	WEAPON_M249 = 14,
	WEAPON_M4A4 = 16,
	WEAPON_MAC10 = 17,
	WEAPON_P90 = 19,
	WEAPON_MP5SD = 23,
	WEAPON_UMP45 = 24,
	WEAPON_XM1014 = 25,
	WEAPON_BIZON = 26,
	WEAPON_MAG7 = 27,
	WEAPON_NEGEV = 28,
	WEAPON_SAWEDOFF = 29,
	WEAPON_TEC9 = 30,
	WEAPON_TASER = 31,
	WEAPON_P2000 = 32,
	WEAPON_MP7 = 33,
	WEAPON_MP9 = 34,
	WEAPON_NOVA = 35,
	WEAPON_P250 = 36,
	WEAPON_SCAR20 = 38,
	WEAPON_SG553 = 39,
	WEAPON_SSG08 = 40,
	WEAPON_KNIFE_CT = 42,
	WEAPON_FLASHBANG = 43,
	WEAPON_HEGRENADE = 44,
	WEAPON_SMOKE = 45,
	WEAPON_MOLOTOV = 46,
	WEAPON_DECOY = 47,
	WEAPON_INCGRENADE = 48,
	WEAPON_C4 = 49,
	WEAPON_M4A1S = 60,
	WEAPON_USPS = 61,
	WEAPON_CZ75 = 63,
	WEAPON_REVOLVER = 64,
	WEAPON_KNIFE_T = 59,

	// Knife skins
	KNIFE_BAYONET = 500,
	KNIFE_CSS = 503,
	KNIFE_FLIP = 505,
	KNIFE_GUT = 506,
	KNIFE_KARAMBIT = 507,
	KNIFE_M9_BAYONET = 508,
	KNIFE_HUNTSMAN = 509,
	KNIFE_FALCHION = 512,
	KNIFE_BOWIE = 514,
	KNIFE_BUTTERFLY = 515,
	KNIFE_SHADOW_DAGGERS = 516,
	KNIFE_PARACORD = 517,
	KNIFE_SURVIVAL = 518,
	KNIFE_URSUS = 519,
	KNIFE_NAVAJA = 520,
	KNIFE_NOMAD = 521,
	KNIFE_STILETTO = 522,
	KNIFE_TALON = 523,
	KNIFE_SKELETON = 525,
};

struct KnifeModel {
	WeaponId id;
	const char* name;
	const char* model;    // view model
	const char* wmodel;   // world model
};

struct SkinInfo {
	int paintKit;
	const char* name;
};

// RecvProxy data structure for intercepting netvar updates
struct CRecvProxyData {
	const void* m_pRecvProp;
	union {
		float	m_Float;
		long	m_Int;
		const char* m_pString;
		void* m_pData;
		int   m_Vector[3];
		int64_t m_Int64;
	} m_Value;
	int m_iElement;
	int m_ObjectID;
};

using RecvVarProxyFn = void(__cdecl*)(const CRecvProxyData* pData, void* pStruct, void* pOut);

namespace skinchanger
{
	// Call from FrameStageNotify at POSTDATAUPDATE_START
	void Run();

	// Force a full update to apply knife model changes
	void ForceUpdate();

	// Install/remove RecvProxy hooks (sequence + fallback skin props)
	void InstallHooks();
	void RemoveHooks();

	// Data lookups
	const std::vector<KnifeModel>& GetKnifeModels();
	const std::vector<SkinInfo>& GetPopularSkins();

	// Check if a weapon def index is a knife
	bool IsKnife(short defIndex);
}
