cmake_minimum_required(VERSION 3.20)
project(CSGO_v2 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force 32-bit (x86) build — CS:GO is a 32-bit process
if(NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM Win32)
endif()

# DirectX SDK path
set(DXSDK_DIR "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")

# ============================================================
# External libraries (ImGui, MinHook, MD5, AnimationLib)
# ============================================================
add_library(ext_imgui STATIC
    ext/ImGui/imgui.cpp
    ext/ImGui/imgui_demo.cpp
    ext/ImGui/imgui_draw.cpp
    ext/ImGui/imgui_impl_dx9.cpp
    ext/ImGui/imgui_impl_win32.cpp
    ext/ImGui/imgui_tables.cpp
    ext/ImGui/imgui_widgets.cpp
)
target_include_directories(ext_imgui PUBLIC ext/ImGui)
target_include_directories(ext_imgui SYSTEM PUBLIC "${DXSDK_DIR}/Include")
target_compile_definitions(ext_imgui PRIVATE WIN32 _WINDOWS)

add_library(ext_minhook STATIC
    ext/MinHook/buffer.cpp
    ext/MinHook/hook.cpp
    ext/MinHook/trampoline.cpp
    ext/MinHook/hde/hde32.cpp
    ext/MinHook/hde/hde64.cpp
)
target_include_directories(ext_minhook PUBLIC ext/MinHook)

add_library(ext_md5 STATIC ext/MD5/MD5.cpp)
target_include_directories(ext_md5 PUBLIC ext/MD5)

add_library(ext_animation STATIC ext/AnimationLib/ext/easing.cpp)
target_include_directories(ext_animation PUBLIC ext/AnimationLib)

# ============================================================
# Main DLL — CSGO_v2
# ============================================================
add_library(CSGO_v2 SHARED
    # Source Files
    CSGO_v2/dllmain.cpp

    # SDK
    CSGO_v2/SDK/interface/interface.cpp
    CSGO_v2/SDK/netvars/netvars.cpp
    CSGO_v2/SDK/Entity/entity.cpp
    CSGO_v2/SDK/Entity/localplayer.cpp
    CSGO_v2/SDK/CRC/CRC.cpp
    CSGO_v2/SDK/Classes/UserCmd/UserCmd.cpp
    CSGO_v2/SDK/Classes/Input/Input.cpp

    # lib
    CSGO_v2/lib/Hooks/hook.cpp
    CSGO_v2/lib/Hooks/GUI/GUI.cpp
    CSGO_v2/lib/Hooks/Manager/hooksManager.cpp
    CSGO_v2/lib/Memory/VMT.cpp
    CSGO_v2/lib/utils/utils.cpp
    CSGO_v2/lib/Configs/config.cpp
    CSGO_v2/lib/Menu/Menu.cpp

    # dx9
    CSGO_v2/dx9/Drawing/drawing.cpp

    # Modules
    CSGO_v2/Modules/Aimbot/aimbot.cpp
    CSGO_v2/Modules/Aimbot/Backtrack.cpp
    CSGO_v2/Modules/Misc/Misc.cpp
    CSGO_v2/Modules/Visuals/ESP.cpp
    CSGO_v2/Modules/Visuals/Crosshair.cpp
    CSGO_v2/Modules/Visuals/Glow.cpp
    CSGO_v2/Modules/Visuals/Hitmarker.cpp
    CSGO_v2/Modules/Visuals/Chams.cpp
    CSGO_v2/Modules/Visuals/SkinChanger.cpp
    CSGO_v2/SDK/Classes/KeyValues.cpp
    CSGO_v2/Modules/Analysis/RuntimeDumper.cpp
)

target_include_directories(CSGO_v2 PRIVATE
    CSGO_v2
    ext/MinHook
    ext/ImGui
    ext/MD5
    ext/AnimationLib
    ext/ImGuiNotify/example/src
    ext/Fonts
)

target_include_directories(CSGO_v2 SYSTEM PRIVATE
    "${DXSDK_DIR}/Include"
)

target_link_directories(CSGO_v2 PRIVATE
    "${DXSDK_DIR}/Lib/x86"
)

target_link_libraries(CSGO_v2 PRIVATE
    ext_imgui
    ext_minhook
    ext_md5
    ext_animation
    d3d9.lib
    d3dx9.lib
    Shell32.lib
)

target_compile_definitions(CSGO_v2 PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    CSGOV2_EXPORTS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# MSVC-specific settings
if(MSVC)
    target_compile_options(CSGO_v2 PRIVATE
        /W3             # Warning level 3
        /permissive-    # Conformance mode
        /Zc:__cplusplus # Report correct C++ standard
    )
    # Disable UAC
    target_link_options(CSGO_v2 PRIVATE /MANIFESTUAC:NO)
endif()

# Character set: MultiByte (not Unicode) for Debug/Release Win32
target_compile_definitions(CSGO_v2 PRIVATE _MBCS)

# ============================================================
# Loader — CSGO_Loader (console app)
# ============================================================
add_executable(CSGO_Loader
    CSGO_Loader/CSGO_Loader.cpp
    CSGO_Loader/Process.cpp
    CSGO_Loader/utils.cpp
)

target_include_directories(CSGO_Loader PRIVATE CSGO_Loader)
target_link_libraries(CSGO_Loader PRIVATE ext_md5)

target_compile_definitions(CSGO_Loader PRIVATE
    WIN32
    _WINDOWS
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

if(MSVC)
    target_compile_options(CSGO_Loader PRIVATE
        /W3
        /permissive-
        /Zc:__cplusplus
    )
endif()

# Force build_timestamp.h to recompile every build (updates __DATE__/__TIME__)
add_custom_command(TARGET CSGO_v2 PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E touch
        "${CMAKE_CURRENT_SOURCE_DIR}/CSGO_v2/build_timestamp.h"
)

# Copy DLL next to loader exe after build so the default path works
add_custom_command(TARGET CSGO_Loader POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:CSGO_v2>
        $<TARGET_FILE_DIR:CSGO_Loader>/CSGO_v2.dll
)
