# CLAUDE.md — Project Context for Claude Code

## Project Overview
CS:GO internal cheat (DLL injection) with ImGui overlay menu, aimbot, ESP, movement hacks.
- **Language**: C++23, MSVC v143, Win32 x86
- **Build**: CMake, Debug builds for injection
- **Target**: CS:GO (32-bit Source Engine process)

## Build Commands
Use the build scripts in `CSGO_v22/CSGO_v2/scripts/`:
```bash
# Build Debug (what gets injected) — USE THIS
cmd.exe /c "CSGO_v22\\CSGO_v2\\scripts\\build-debug.bat"

# Build Release
cmd.exe /c "CSGO_v22\\CSGO_v2\\scripts\\build-release.bat"

# Clean build
cmd.exe /c "CSGO_v22\\CSGO_v2\\scripts\\clean.bat"

# Reconfigure CMake
cmd.exe /c "CSGO_v22\\CSGO_v2\\scripts\\configure.bat"
```
- Scripts auto-configure if build dir doesn't exist
- ALWAYS prefer scripts over manual cmake commands

## Key Architecture

### Directory Structure
```
CSGO_v22/CSGO_v2/
  CSGO_v2/              # Main DLL source
    lib/
      Menu/Menu.h|.cpp  # Menu framework (GroupBuilder fluent API)
      Hooks/GUI/GUI.h|.cpp  # D3D9 hook, window setup, menu rendering
      Hooks/hook.cpp    # VMT hooks (EndScene, CreateMove, etc.)
      Configs/config.h|.cpp  # Config save/load with MD5 checksums
      Error/Log.h       # Logging system
    Modules/
      Aimbot/           # Aimbot module
      Visuals/ESP.cpp   # ESP rendering
      Misc/             # Movement hacks (bhop, strafe, etc.)
    SDK/                # Game SDK (interfaces, netvars, entities)
    dx9/Drawing/        # D3D9 drawing helpers
  CSGO_Loader/          # Injector executable
  ext/                  # External libs (ImGui 1.88, MinHook, MD5, AnimationLib)
```

### Menu Framework (`menu::` namespace)
- **GroupBuilder**: Fluent chainable API for declaring menu content
  ```cpp
  menu::LeftGroup("Name", lineCount)
      .Checkbox("Label", &value)
      .Slider("Label", &value, min, max)
      .End();
  ```
- **Sections**: `menu::Section("Name")` for outline groups at column width
- **SubSection**: Nested outline groups inside groups via `.SubSection(name, lambda)`
- **Custom**: `.Custom(lambda)` escape hatch for raw ImGui calls
- **ListBox**: `.ListBox(label, id, items, selected, onSelect)` built-in
- All ImGui calls isolated in `detail::` namespace (Menu.cpp) for easy restyling
- Column width centralized as `menu::kColumnWidth` (220px, reduced from 270 for sidebar layout)
- Tab transition state: `g_tabAlpha`, `g_groupIndex`, `g_tabSwitchTime` for animated tab switching
- Sidebar layout: 160px sidebar + content area with two 220px columns

### Config System
- INI-style key=value format with section headers
- MD5 checksum in first line: `# CSGO_v2 cfg <hash>`
- Only verified configs are listed in the menu
- Config dir: `C:\Users\adama\Documents\CSGO_v2_Configs`

### Logging
- Log files at: `C:\Users\adama\Documents\CSGO_v2_Logs`
- `csgo_v2.log` — general log, `csgo_v2_errors.log` — error dumps

### ImGui Version & Critical Rules
- **ImGui 1.88 WIP** (version 18724)
- NO auto-resize child windows (that's 1.89+)
- `BeginChild` with height 0 fills available space, cannot shrink to content
- Line-count based height calculation for groups
- **Side-by-side layout**: LeftGroup/RightGroup use independent Y cursor tracking per column via `SetCursorPos`. Each column stacks groups without gaps. `SetCursorPos` going backwards is safe — `CursorMaxPos` only grows (imgui.cpp:8380). Call `menu::EndRow()` before leaving a tab to finalize.
- ImGui source/docs at `ext/ImGui/` — `imgui_demo.cpp` is the best reference for layout patterns
- **⚠️ IMPORTANT**: See memory files `technical-imgui-patterns.md` and `menu-architecture.md` for comprehensive ImGui patterns, alpha stacking, custom widget recipes, and animation system details. These MUST be read before doing UI work (user feedback given twice).

### ImGui Alpha System (CRITICAL — causes bugs if misunderstood)
- `GetStyle().Alpha` is GLOBAL — we set it to `windowFade.getValue()` in Render()
- `PushStyleVar(Alpha, val)` REPLACES alpha, doesn't multiply. Compute manually: `GetStyle().Alpha * myFactor`
- **DrawList functions IGNORE style alpha** — use `GetColorU32(ImVec4)` which DOES multiply by style.Alpha
- Our alpha stack: windowFade (global) → tabAlpha (content push) → groupAlpha (per-group push)
- PushStyleVar/PopStyleVar MUST be perfectly balanced or ImGui will crash/corrupt state

## Conventions
- `cfg` is the global `Config` instance (defined in config.h as `inline Config cfg`)
- `globals::g_interfaces` holds all game interface pointers
- `hooks::` namespace for hook-related globals
- Use `menu::` namespace for all menu widgets, never raw ImGui in GUI.cpp render code
- LNK1168 error = DLL still loaded in game, must unload first
- Debug builds are what gets injected, not Release

## Menu Design Rules
- **Balance left/right columns**: ALWAYS distribute groups so left and right sides are roughly equal height. Never let one side be significantly taller.
- **Inline color pickers**: Put color pickers on the SAME LINE as the checkbox they belong to using `.SameLine().ColorPicker("##hiddenId", color, true)`. The `##` prefix hides the label text.
- **GearPopup for sub-options**: Features with extra settings use a gear icon popup (`.GearPopup("id", lambda)`), not separate rows.
- **Group sizing**: Use the minimum `lines` count that fits the content. Keep groups compact.
- **Toggle style**: iOS-style toggle switches by default (configurable via Settings > Toggle Style).

## External References
- `external_sources/Osiris-csgo/` — Open-source CSGO cheat (C++). Can be used as a reference when stuck.
- `external_sources/csgo-2018-source-main/` — Valve's official CSGO 2018 source code. Useful for SDK/engine internals.
- **IMPORTANT**: These are references ONLY. Always find a BETTER approach first before copying patterns from Osiris. The goal is for this cheat to be superior in code quality, design, and features. Only fall back to reference code when there's no clearly better alternative.

## Static Analysis Toolkit
- **Script**: `tools/analyze_dll.py` — PE parser, RTTI extractor, vtable mapper, interface finder
- **CS:GO path**: `D:\SteamLibrary\steamapps\common\Counter-Strike Global Offensive\`
- **Key commands**:
  ```bash
  python tools/analyze_dll.py --interfaces      # All interface version strings
  python tools/analyze_dll.py --netvars          # NetVar properties in client.dll
  python tools/analyze_dll.py --xref             # Validate SDK vtable indices
  python tools/analyze_dll.py --find-class Name  # RTTI + vtable for a class
  python tools/analyze_dll.py --convars engine   # ConVar strings
  ```
- **WSL Kali** has `objdump`, `strings`, `readelf`, `nm` for deeper binary analysis

## Debugging Techniques
- **Crash analysis via binary disassembly**: When VEH reports a crash offset, use WSL Kali:
  ```bash
  wsl -d kali-linux -- objdump -d -M intel --start-address=$((0x10000000 + OFFSET)) --stop-address=$((0x10000000 + OFFSET + 0x40)) '/mnt/c/.../CSGO_v2.dll'
  ```
  - DLL image base: `0x10000000`. Crash offset from VEH = RVA
  - Find callers: `objdump | grep 'call.*<target_address>'`, check jmp thunk tables
  - Trace the null pointer to identify which game entity/virtual call returned null
- **Build timestamp**: `build_timestamp.h` is force-touched by CMake pre-build step. Init notification shows `"CSGO_v2 loaded [<date> - <time>]"` to confirm new DLL injection
- **MSVC debug `= {}` pitfall**: `math::Vector v = {};` compiles as copy-from-NULL in debug. Use `math::Vector v(0.f, 0.f, 0.f);` and `v.x = 0.f; v.y = 0.f; v.z = 0.f;` for reset

## Common Pitfalls
- Forward declare functions used before definition in .cpp files (C3861)
- ImGui outline groups don't create content regions — widgets use window width
- Always pass explicit width to outline groups or use `menu::Section()` which defaults to kColumnWidth
- `ListBox` width `0` uses `CalcItemWidth()`, `-FLT_MIN` fills available (too wide in outline groups)
- **Fixed vs scrollable elements**: Title bar / sidebar MUST be outside scrollable child windows. Rendering them via `GetWindowDrawList()` inside a scrollable area clips them on scroll. This bug occurred TWICE.
- **DisplayName() with ## prefix**: `"##FOV"` has empty display name since it starts with `##`. Use `"FOV##slider_FOV"` format so DisplayName extracts "FOV".
- **`std::sin()` returns double**: Cast result to float to avoid C4244: `std::sinf((float)g.Time * 6.0f)`
- **After any UI work**: Update memory files (`technical-imgui-patterns.md`, `menu-architecture.md`) with new patterns discovered
