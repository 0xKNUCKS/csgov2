# CS:GO Feature Research — Implementation Reference

## Priority Features (by impact vs effort)

| Feature | Impact | Effort | Status |
|---------|--------|--------|--------|
| **Radar Hack** | High | Trivial | Done |
| **Backtracking** | Very High | Medium | Done |
| **Fake Lag** | High | Low | Done |
| **Glow ESP** | High | Low | Done |
| **Hitmarker** | Medium | Low | Done |
| **Skin Changer** | Medium | Medium | Not started |
| **Chams** | High | Medium | Not started |
| **Night Mode** | Low-Med | Low | Done |
| **Auto-Stop** | Medium | Low | Done |
| **Grenade Prediction** | Medium | High | Not started |
| **Anti-Aim** | High (HvH) | Medium | Not started |
| **Resolver** | High (HvH) | Very High | Not started |
| **Sound ESP** | Low-Med | Medium | Not started |

---

## 1. RADAR HACK (Trivial)
Set `m_bSpotted = true` on all enemy entities each tick in CreateMove.
```cpp
*(bool*)((uintptr_t)ent + offsets::m_bSpotted) = true;
```
Already have `m_bSpotted` offset. Simplest possible feature.

---

## 2. BACKTRACKING (Medium effort, Very High impact)
Shoot players at positions up to 200ms ago (12 ticks at 64-tick). Server accepts due to lag compensation.

**Per-tick storage:**
```cpp
struct BacktrackRecord {
    float simtime;
    math::Vector origin;
    math::Vector head;       // bone[8] position
    math::Vector absAngles;
    math::Matrix3x4 boneMatrix[128];
    bool valid;
};
// Per-player: std::deque<BacktrackRecord> records[65];
```

**Key netvars:** `m_flSimulationTime` (DT_BaseEntity), `m_flOldSimulationTime`, `m_nTickBase` (DT_BasePlayer)

**Validity check:**
```cpp
bool IsTickValid(float simtime) {
    auto nci = Engine->GetNetChannelInfo(); // vfunc 78 on IVEngineClient
    float correct = clamp(nci->GetLatency(0) + nci->GetLatency(1) +
                          TICKS_TO_TIME(GlobalVars->client_tick - GlobalVars->server_tick), 0.f, 0.2f);
    float deltaTime = correct - (GlobalVars->curtime - simtime);
    return abs(deltaTime) <= 0.2f;
}
```

**Usage in aimbot:** Iterate backtrack records alongside current positions. If best target is a backtrack record, set `cmd->tick_count = TIME_TO_TICKS(record.simtime)`.

**Need:** `INetChannelInfo` from `Engine->GetNetChannelInfo()` (vfunc 78).

---

## 3. FAKE LAG (Low effort, High impact)
Choke network packets via `bSendPacket` (already accessible in hkCreateMove proxy).
```cpp
static int chokedTicks = 0;
if (cfg.misc.FakeLag && chokedTicks < cfg.misc.FakeLagAmount) {
    *bSendPacket = false;
    chokedTicks++;
} else {
    *bSendPacket = true;
    chokedTicks = 0;
}
```
Server limit: 15 choked ticks max. Player teleports from enemy perspective.

---

## 4. GLOW ESP (Low effort, High impact)
Find `IGlowObjectManager` via sig in `client.dll`: `0F 11 05 ? ? ? ? 83 C8 01`

**Glow structure:**
```cpp
struct GlowObjectDefinition_t {
    gEntity* pEntity;           // 0x00
    float r, g, b, a;           // 0x04-0x10
    char pad[8];                // 0x14
    float bloomAmount;          // 0x1C (set 1.0)
    char pad2[4];               // 0x20
    bool renderWhenOccluded;    // 0x24 (true = wallhack)
    bool renderWhenUnoccluded;  // 0x25
    bool fullBloomRender;       // 0x26
    char pad3[5];               // 0x27
    int splitScreenSlot;        // 0x2C
    int nextFreeSlot;           // 0x30
};
```
Run in CreateMove or FrameStageNotify (FRAME_NET_UPDATE_POSTDATAUPDATE_START).

---

## 5. HITMARKER (Low effort, Medium impact)
Use `IGameEventManager2` ("GAMEEVENTSMANAGER002" from engine.dll).
Listen for `"player_hurt"` event. Fields: `userid`, `attacker`, `health`, `dmg_health`, `hitgroup`.

```cpp
class GameEventListener : public IGameEventListener2 {
    void FireGameEvent(IGameEvent* event) override {
        if (strcmp(event->GetName(), "player_hurt") == 0) {
            int attacker = event->GetInt("attacker");
            int damage = event->GetInt("dmg_health");
            int hitgroup = event->GetInt("hitgroup");
            // Record hit for rendering
        }
    }
    int GetEventDebugID() override { return 42; }
};
// Register: gameEventManager->AddListener(&listener, "player_hurt", false);
```
Render: 4 lines at 45deg from crosshair center, fade over ~0.3s. Show damage numbers.

---

## 6. SKIN CHANGER (Medium effort)
Hook `FrameStageNotify` at `FRAME_NET_UPDATE_POSTDATAUPDATE_START`.

**Netvars to modify per weapon:**
- `m_iItemDefinitionIndex` — weapon ID (42=CT knife, 500=Bayonet, 505=M9, 507=Karambit)
- `m_nFallbackPaintKit` — skin ID (309=Crimson Web, 344=Howl)
- `m_nFallbackSeed` — pattern seed
- `m_flFallbackWear` — wear float (0.0=FN)
- `m_nFallbackStatTrak` — stattrak counter (-1=none)
- `m_iItemIDHigh` — set to -1 to force fallback values
- `m_iEntityQuality` — 3 for knives
- `m_szCustomName` — nametag (32 chars)

After modifying, call `setModelIndex(newModelIndex)` and `postDataUpdate(0)`.
Knife models: `models/weapons/v_knife_karam.mdl`, `v_knife_butterfly.mdl`, etc.
May need `Engine->ClientCmd_Unrestricted("fullupdate")`.

---

## 7. CHAMS (Medium effort, High impact)
Hook `DrawModelExecute` on `IVModelRender` (vfunc 21, "VEngineModel016" from engine.dll).
Check model name for `"models/player"`. Call `ForcedMaterialOverride(material)` before original.

Create flat unlit material via `IMaterialSystem::CreateMaterial()`:
```
"UnlitGeneric" { "$basetexture" "vgui/white_additive" "$ignorez" "1" "$nofog" "1" "$model" "1" "$selfillum" "1" "$flat" "1" }
```
Need: `IMaterialSystem` ("VMaterialSystem080" from materialsystem.dll), `IStudioRender` ("VStudioRender026").

---

## 8. NIGHT MODE (Low effort)
Use `IMaterialSystem` to iterate materials and `ColorModulate(r,g,b)` world/skybox materials.
Or simpler: ConVar `mat_fullbright 1`.
IMaterialSystem vfuncs: FirstMaterial=86, NextMaterial=87, InvalidMaterial=88, GetMaterial=89.

---

## 9. AUTO-STOP (Low effort, Medium impact)
In CreateMove when attack button pressed:
```cpp
auto velocity = lp->getVelocity();
float speed = velocity.length2D();
if (speed > 1.f) {
    float direction = atan2f(velocity.y, velocity.x) * RAD_TO_DEG;
    float delta = direction - cmd->viewangles.y;
    cmd->forwardmove = -cosf(delta * DEG_TO_RAD) * 450.f;
    cmd->sidemove = sinf(delta * DEG_TO_RAD) * 450.f;
}
```

---

## 10. ANTI-AIM (Medium effort, HvH)
Manipulate `cmd->viewangles` on choked ticks (`bSendPacket = false`) for desync.
Max desync ~58 degrees. Read `m_flLowerBodyYawTarget` (DT_CSPlayer) for LBY updates.
Common: pitch down, yaw jitter, sideways body.

---

## 11. GRENADE PREDICTION (High effort)
Simulate projectile physics: parabolic with gravity (sv_gravity=800), air drag, bounces.
Bounce elasticity: HE~0.45, Flash~0.2, Smoke~0.65.
Throw speed ~750 u/s. Use TraceRay for surface detection. Render polyline path.
Detonation: HE/Flash=1.5s, Smoke=3.5s, Molotov=on ground impact.

---

## 12. SOUND ESP (Medium effort)
Hook `EmitSound` on `IEngineSound` ("IEngineSoundClient003", vfunc 5).
Filter `"player/footsteps"` sounds. Store 3D origin + timestamp, render directional indicator.
Alternative: `IGameEventManager2` -> `"player_footstep"` event.

---

## 13. SPECTATOR LIST (Low effort)
Read `m_hObserverTarget` netvar on dead players. If it matches local player handle, they're spectating you.
Also check `m_iObserverMode` for spectator mode type. Render as overlay list.

---

## 14. WEAPON ESP (Low effort)
Iterate entities beyond maxClients. Check ClientClass for weapon classes or `isWeapon()`.
WorldToScreen origin and draw weapon name.

---

## New Interfaces Needed
```cpp
IMaterialSystem* MaterialSystem = nullptr;    // "VMaterialSystem080" from materialsystem.dll
IVModelRender* ModelRender = nullptr;          // "VEngineModel016" from engine.dll
IGameEventManager2* GameEventMgr = nullptr;    // "GAMEEVENTSMANAGER002" from engine.dll
IEngineSound* EngineSound = nullptr;           // "IEngineSoundClient003" from engine.dll
```

---

## Anti-Cheat Notes
- Our MinHook detours are safer than VMT hooks (VAC checks vtable pointers)
- Manual mapping avoids module enumeration
- Don't write to sv_cheats-protected ConVars directly
- Consider XOR string encryption for hardcoded strings
- Zero PE headers after init
- Avoid GetProcAddress on game modules
