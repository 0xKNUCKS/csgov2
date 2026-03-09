# CLAUDE.md — Project Context for Claude Code

## Project Overview
CS:GO internal cheat (DLL injection) with ImGui overlay menu, aimbot, ESP, movement hacks.
- **Language**: C++23, MSVC v143, Win32 x86
- **Build**: CMake, Debug builds for injection
- **Target**: CS:GO (32-bit Source Engine process)

## Build Commands
```bash
# CMake path (VS 18 Insiders)
CMAKE="/c/Program Files/Microsoft Visual Studio/18/Insiders/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"

# Configure (only needed once)
"$CMAKE" -S CSGO_v22/CSGO_v2 -B CSGO_v22/CSGO_v2/build -A Win32

# Build Debug (what gets injected)
"$CMAKE" --build CSGO_v22/CSGO_v2/build --config Debug

# Build Release
"$CMAKE" --build CSGO_v22/CSGO_v2/build --config Release
```

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
- Column width centralized as `menu::kColumnWidth` (270px)

### Config System
- INI-style key=value format with section headers
- MD5 checksum in first line: `# CSGO_v2 cfg <hash>`
- Only verified configs are listed in the menu
- Config dir: `C:\Users\adama\Documents\CSGO_v2_Configs`

### Logging
- Log files at: `C:\Users\adama\Documents\CSGO_v2_Logs`
- `csgo_v2.log` — general log, `csgo_v2_errors.log` — error dumps

### ImGui Version
- **ImGui 1.88 WIP** (version 18724)
- NO auto-resize child windows (that's 1.89+)
- `BeginChild` with height 0 fills available space, cannot shrink to content
- Line-count based height calculation for groups
- **Side-by-side layout**: LeftGroup/RightGroup wrap children in `BeginGroup()`/`EndGroup()`. `EndGroup()` tracks bounding box so the next row starts below the taller group. See `imgui_demo.cpp:7033-7073`.
- ImGui source/docs at `ext/ImGui/` — `imgui_demo.cpp` is the best reference for layout patterns

## Conventions
- `cfg` is the global `Config` instance (defined in config.h as `inline Config cfg`)
- `globals::g_interfaces` holds all game interface pointers
- `hooks::` namespace for hook-related globals
- Use `menu::` namespace for all menu widgets, never raw ImGui in GUI.cpp render code
- LNK1168 error = DLL still loaded in game, must unload first
- Debug builds are what gets injected, not Release

## Common Pitfalls
- Forward declare functions used before definition in .cpp files (C3861)
- ImGui outline groups don't create content regions — widgets use window width
- Always pass explicit width to outline groups or use `menu::Section()` which defaults to kColumnWidth
- `ListBox` width `0` uses `CalcItemWidth()`, `-FLT_MIN` fills available (too wide in outline groups)
