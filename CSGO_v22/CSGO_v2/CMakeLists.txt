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
    CSGO_v2/dllmain.cpp
    CSGO_v2/hook.cpp
    CSGO_v2/hooksManager.cpp
    CSGO_v2/VMT.cpp
    CSGO_v2/GUI.cpp
    CSGO_v2/drawing.cpp
    CSGO_v2/UI.cpp
    CSGO_v2/ESP.cpp
    CSGO_v2/aimbot.cpp
    CSGO_v2/Misc.cpp
    CSGO_v2/entity.cpp
    CSGO_v2/localplayer.cpp
    CSGO_v2/interface.cpp
    CSGO_v2/netvars.cpp
    CSGO_v2/utils.cpp
    CSGO_v2/Input.cpp
    CSGO_v2/UserCmd.cpp
    CSGO_v2/CRC.cpp
    CSGO_v2/config.cpp
)

target_include_directories(CSGO_v2 PRIVATE
    CSGO_v2
    ext/MinHook
    ext/ImGui
    ext/MD5
    ext/AnimationLib
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
