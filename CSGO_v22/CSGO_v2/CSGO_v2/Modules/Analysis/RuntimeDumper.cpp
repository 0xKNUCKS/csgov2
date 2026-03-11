#include "RuntimeDumper.h"
#include "SDK/Globals/Globals.h"
#include "SDK/Entity/entity.h"
#include "SDK/Entity/localplayer.h"
#include "lib/Error/Log.h"

#include <fstream>
#include <sstream>
#include <format>
#include <filesystem>
#include <ShlObj.h>

namespace analysis {

    // Simple JSON writer (no external deps)
    class JsonWriter {
        std::ofstream m_file;
        int m_indent = 0;
        bool m_needComma = false;

        void writeIndent() {
            for (int i = 0; i < m_indent; i++) m_file << "  ";
        }
        void maybeComma() {
            if (m_needComma) m_file << ",";
            m_file << "\n";
        }
        // Escape special characters for valid JSON strings
        static std::string escapeJson(const std::string& s) {
            std::string out;
            out.reserve(s.size());
            for (char c : s) {
                switch (c) {
                    case '"':  out += "\\\""; break;
                    case '\\': out += "\\\\"; break;
                    case '\n': out += "\\n";  break;
                    case '\r': out += "\\r";  break;
                    case '\t': out += "\\t";  break;
                    default:   out += c;      break;
                }
            }
            return out;
        }
    public:
        bool open(const std::string& path) {
            m_file.open(path);
            return m_file.is_open();
        }
        void close() { m_file << "\n"; m_file.close(); }

        void beginObject() {
            maybeComma();
            writeIndent();
            m_file << "{";
            m_indent++;
            m_needComma = false;
        }
        void beginObjectInline() {
            m_file << "{";
            m_indent++;
            m_needComma = false;
        }
        void endObject() {
            m_indent--;
            m_file << "\n";
            writeIndent();
            m_file << "}";
            m_needComma = true;
        }
        void beginArray(const char* key) {
            maybeComma();
            writeIndent();
            m_file << "\"" << key << "\": [";
            m_indent++;
            m_needComma = false;
        }
        void endArray() {
            m_indent--;
            m_file << "\n";
            writeIndent();
            m_file << "]";
            m_needComma = true;
        }
        void key(const char* k) {
            maybeComma();
            writeIndent();
            m_file << "\"" << k << "\": ";
            m_needComma = false;
        }
        void keyVal(const char* k, const std::string& v) {
            maybeComma();
            writeIndent();
            m_file << "\"" << escapeJson(k) << "\": \"" << escapeJson(v) << "\"";
            m_needComma = true;
        }
        void keyVal(const char* k, const char* v) {
            keyVal(k, std::string(v));
        }
        void keyVal(const char* k, int v) {
            maybeComma();
            writeIndent();
            m_file << "\"" << k << "\": " << v;
            m_needComma = true;
        }
        void keyVal(const char* k, float v) {
            maybeComma();
            writeIndent();
            m_file << "\"" << k << "\": " << v;
            m_needComma = true;
        }
        void keyHex(const char* k, uintptr_t v) {
            maybeComma();
            writeIndent();
            m_file << "\"" << k << "\": \"" << std::format("0x{:08X}", v) << "\"";
            m_needComma = true;
        }
    };

    std::string GetOutputDir() {
        char docs[MAX_PATH];
        SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, docs);
        std::string dir = std::string(docs) + "\\CSGO_v2_Analysis\\dynamic";
        std::filesystem::create_directories(dir);
        return dir;
    }

    // -----------------------------------------------------------------------
    // Netvar Dump — walks the entire ClientClass chain
    // -----------------------------------------------------------------------
    static void DumpRecvTable(JsonWriter& jw, RecvTable* table, int depth) {
        if (!table || depth > 10) return;

        for (int i = 0; i < table->m_nProps; i++) {
            RecvProp& prop = table->m_pProps[i];
            if (!prop.m_pVarName) continue;

            // Skip baseclass entries to avoid noise
            if (strcmp(prop.m_pVarName, "baseclass") == 0) continue;

            jw.beginObject();
            jw.keyVal("name", prop.m_pVarName);
            jw.keyVal("offset", prop.m_Offset);

            // Prop type (m_RecvType is void* in our SDK, cast to int for the enum)
            int recvType = (int)(uintptr_t)prop.m_RecvType;
            const char* typeStr = "unknown";
            switch (recvType) {
                case 0: typeStr = "int"; break;
                case 1: typeStr = "float"; break;
                case 2: typeStr = "vector"; break;
                case 3: typeStr = "vectorxy"; break;
                case 4: typeStr = "string"; break;
                case 5: typeStr = "array"; break;
                case 6: typeStr = "datatable"; break;
                case 7: typeStr = "int64"; break;
            }
            jw.keyVal("type", typeStr);

            // Recurse into nested datatables
            if (prop.m_pDataTable && recvType == 6) {
                jw.keyVal("table", prop.m_pDataTable->m_pNetTableName ?
                          prop.m_pDataTable->m_pNetTableName : "?");
                jw.beginArray("props");
                DumpRecvTable(jw, prop.m_pDataTable, depth + 1);
                jw.endArray();
            }

            jw.endObject();
        }
    }

    void DumpNetvars() {
        auto* classes = globals::g_interfaces.BaseClient->GetAllClasses();
        if (!classes) {
            Log::Err("Analysis", "No ClientClasses available");
            return;
        }

        std::string path = GetOutputDir() + "\\netvars.json";
        JsonWriter jw;
        if (!jw.open(path)) {
            Log::Err("Analysis", "Failed to open {}", path);
            return;
        }

        jw.beginObjectInline();
        jw.beginArray("classes");

        int classCount = 0;
        for (auto* cc = classes; cc; cc = cc->m_pNext) {
            if (!cc->m_pRecvTable) continue;

            jw.beginObject();
            jw.keyVal("name", cc->m_pNetworkName ? cc->m_pNetworkName : "?");
            jw.keyVal("table", cc->m_pRecvTable->m_pNetTableName ?
                      cc->m_pRecvTable->m_pNetTableName : "?");
            jw.keyVal("class_id", cc->m_ClassID);

            jw.beginArray("props");
            DumpRecvTable(jw, cc->m_pRecvTable, 0);
            jw.endArray();

            jw.endObject();
            classCount++;
        }

        jw.endArray();
        jw.keyVal("total_classes", classCount);
        jw.endObject();
        jw.close();

        Log::Info("Analysis", "Dumped {} ClientClasses to {}", classCount, path.c_str());
    }

    // -----------------------------------------------------------------------
    // Interface Dump — our captured interface pointers + vtable info
    // -----------------------------------------------------------------------
    void DumpInterfaces() {
        std::string path = GetOutputDir() + "\\interfaces.json";
        JsonWriter jw;
        if (!jw.open(path)) return;

        jw.beginObjectInline();
        jw.beginArray("interfaces");

        auto dumpIface = [&](const char* name, const char* version, void* ptr) {
            jw.beginObject();
            jw.keyVal("name", name);
            jw.keyVal("version", version);
            jw.keyHex("address", (uintptr_t)ptr);
            if (ptr) {
                // Read vtable pointer (first dword of object)
                uintptr_t vtable = *(uintptr_t*)ptr;
                jw.keyHex("vtable", vtable);

                // Count vtable entries (valid code pointers)
                MEMORY_BASIC_INFORMATION mbi;
                int count = 0;
                uintptr_t* vt = (uintptr_t*)vtable;
                for (int i = 0; i < 600; i++) {
                    if (!VirtualQuery((void*)vt[i], &mbi, sizeof(mbi))) break;
                    if (!(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ |
                          PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) break;
                    count++;
                }
                jw.keyVal("vfunc_count", count);
            }
            jw.endObject();
        };

        auto& ifaces = globals::g_interfaces;
        dumpIface("IClientEntityList", "VClientEntityList003", ifaces.ClientEntity);
        dumpIface("IBaseClientDLL", "VClient018", ifaces.BaseClient);
        dumpIface("IVEngineClient", "VEngineClient014", ifaces.Engine);
        dumpIface("CInputSystem", "InputSystemVersion001", ifaces.InputSystem);
        dumpIface("IVModelInfo", "VModelInfoClient004", ifaces.ModelInfo);
        dumpIface("IEngineTrace", "EngineTraceClient004", ifaces.EngineTrace);
        dumpIface("ICvar", "VEngineCvar007", ifaces.Cvar);
        dumpIface("ISurface", "VGUI_Surface031", ifaces.Surface);

        jw.endArray();
        jw.endObject();
        jw.close();

        Log::Info("Analysis", "Dumped interfaces to {}", path.c_str());
    }

    // -----------------------------------------------------------------------
    // Entity Info — local player structure, weapon, key offsets
    // -----------------------------------------------------------------------
    void DumpEntityInfo() {
        std::string path = GetOutputDir() + "\\entity_info.json";
        JsonWriter jw;
        if (!jw.open(path)) return;

        jw.beginObjectInline();

        // Our resolved netvar offsets
        jw.beginArray("resolved_offsets");
        auto dumpOffset = [&](const char* name, uintptr_t val) {
            jw.beginObject();
            jw.keyVal("name", name);
            jw.keyHex("offset", val);
            jw.keyVal("decimal", (int)val);
            jw.endObject();
        };
        dumpOffset("m_bSpotted", offsets::m_bSpotted);
        dumpOffset("m_iTeamNum", offsets::m_iTeamNum);
        dumpOffset("m_iKills", offsets::m_iKills);
        dumpOffset("m_fFlags", offsets::m_fFlags);
        dumpOffset("m_bIsScoped", offsets::m_bIsScoped);
        dumpOffset("deadflag", offsets::deadFlag);
        dumpOffset("m_vecVelocity", offsets::m_vecVelocity);
        dumpOffset("m_vecViewOffset", offsets::m_vecViewOffset);
        dumpOffset("m_aimPunchAngle", offsets::m_aimPunchAngle);
        jw.endArray();

        // If in-game, dump local player info
        if (globals::g_interfaces.Engine && globals::g_interfaces.Engine->IsInGame()) {
            gEntity* lp = LocalPlayer.Get();
            if (lp) {
                jw.key("local_player");
                jw.beginObjectInline();
                jw.keyHex("address", (uintptr_t)lp);

                // VTable pointer
                uintptr_t vtable = *(uintptr_t*)lp;
                jw.keyHex("vtable", vtable);

                // Basic info
                jw.keyVal("health", lp->health());
                jw.keyVal("team", (int)lp->getTeamID());
                jw.keyVal("alive", lp->isAlive() ? 1 : 0);
                jw.keyVal("index", lp->index());

                // Bone cache info
                auto& bones = lp->boneCache();
                jw.keyVal("bone_cache_count", bones.size);
                jw.keyHex("bone_cache_addr", (uintptr_t)bones.memory);
                jw.keyHex("bone_cache_offset", 0x2914);

                // Dump first 16 bytes of entity to see vtable + padding
                jw.beginArray("first_32_bytes_hex");
                uint8_t* raw = (uint8_t*)lp;
                for (int i = 0; i < 32; i += 4) {
                    jw.beginObject();
                    jw.keyHex("offset", i);
                    jw.keyHex("value", *(uintptr_t*)(raw + i));
                    jw.endObject();
                }
                jw.endArray();

                jw.endObject();
            }
        }

        // Module bases (for RVA calculation)
        jw.beginArray("modules");
        auto dumpModule = [&](const char* name) {
            HMODULE h = GetModuleHandleA(name);
            if (h) {
                jw.beginObject();
                jw.keyVal("name", name);
                jw.keyHex("base", (uintptr_t)h);
                jw.endObject();
            }
        };
        dumpModule("client.dll");
        dumpModule("engine.dll");
        dumpModule("vstdlib.dll");
        dumpModule("materialsystem.dll");
        dumpModule("vguimatsurface.dll");
        dumpModule("inputsystem.dll");
        dumpModule("studiorender.dll");
        jw.endArray();

        jw.endObject();
        jw.close();

        Log::Info("Analysis", "Dumped entity info to {}", path.c_str());
    }

    // -----------------------------------------------------------------------
    // ConVar Dump — iterate ConVar linked list
    // -----------------------------------------------------------------------
    void DumpConVars() {
        if (!globals::g_interfaces.Cvar) return;

        std::string path = GetOutputDir() + "\\convars.json";
        JsonWriter jw;
        if (!jw.open(path)) return;

        jw.beginObjectInline();
        jw.beginArray("convars");

        // Walk the ConVar linked list via ICvar
        // ICvar::GetCommands() is typically vfunc index ~19
        // Each ConCommandBase has m_pNext, m_pszName, m_pszHelpString, m_nFlags
        // ConVar extends ConCommandBase with m_pszDefaultValue, m_Value
        //
        // For safety, we use FindVar to check known important convars
        const char* importantCvars[] = {
            "sv_cheats", "cl_interp", "cl_interp_ratio", "cl_updaterate",
            "cl_cmdrate", "rate", "sv_maxrate", "sv_minrate",
            "sv_maxunlag", "sv_maxupdaterate", "sv_minupdaterate",
            "cl_predict", "cl_predictweapons", "cl_lagcompensation",
            "sv_competitive_minspec", "sv_enablebunnyhopping",
            "weapon_accuracy_nospread", "weapon_recoil_scale",
            "sv_showimpacts", "sv_showlagcompensation",
            "mp_autoteambalance", "mp_freezetime", "mp_roundtime",
            "sv_accelerate", "sv_airaccelerate", "sv_friction",
            "sv_gravity", "sv_maxspeed", "sv_stopspeed",
            "cl_crosshairalpha", "cl_crosshaircolor",
            "sensitivity", "zoom_sensitivity_ratio_mouse",
            "net_maxroutable", "cl_fullupdate_predicted_origin_fix",
            "mat_fullbright", "r_drawothermodels",
        };

        int count = 0;
        for (const char* name : importantCvars) {
            ConVar* cv = globals::g_interfaces.Cvar->FindVar(name);
            if (cv) {
                jw.beginObject();
                jw.keyVal("name", name);
                jw.keyHex("address", (uintptr_t)cv);

                // ConCommandBase layout (x86):
                // +0x00: vtable
                // +0x04: m_pNext
                // +0x08: m_bRegistered
                // +0x0C: m_pszName
                // +0x10: m_pszHelpString
                // +0x14: m_nFlags
                uintptr_t base = (uintptr_t)cv;
                int flags = *(int*)(base + 0x14);
                jw.keyVal("flags", flags);

                // ConVar extends ConCommandBase:
                // +0x2C: m_fValue (float)
                // +0x30: m_nValue (int)
                float fval = cv->GetFloat();
                int ival = cv->GetInt();
                jw.keyVal("float_value", fval);
                jw.keyVal("int_value", ival);

                jw.endObject();
                count++;
            }
        }

        jw.endArray();
        jw.keyVal("total_checked", (int)(sizeof(importantCvars) / sizeof(importantCvars[0])));
        jw.keyVal("total_found", count);
        jw.endObject();
        jw.close();

        Log::Info("Analysis", "Dumped {} convars to {}", count, path.c_str());
    }

    // -----------------------------------------------------------------------
    // Dump All
    // -----------------------------------------------------------------------
    void DumpAll() {
        Log::Info("Analysis", "Starting runtime analysis dump...");

        DumpNetvars();
        DumpInterfaces();
        DumpEntityInfo();
        DumpConVars();

        Log::Info("Analysis", "Runtime dump complete -> {}", GetOutputDir().c_str());
    }
}
