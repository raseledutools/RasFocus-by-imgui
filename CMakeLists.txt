cmake_minimum_required(VERSION 3.20)
project(RasFocus VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── ImGui (FetchContent) ───────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.9b
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# ── ImGui static library ───────────────────────────────────────
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# ── Minimal WinMain stub (no globals – they live in tab_adult.cpp) ──
set(STUB_MAIN "${CMAKE_CURRENT_BINARY_DIR}/stub_main.cpp")
file(WRITE "${STUB_MAIN}" [[
// CI stub: provides only the Windows entry point.
// g_isPremiumUser, g_showUpgradePopup, g_parentForceAdultBlock
// are already defined in tab_adult.cpp – do NOT redefine here.
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return 0; }
]])

# ── RasFocus executable ────────────────────────────────────────
add_executable(RasFocus WIN32
    tab_adult.cpp
    "${STUB_MAIN}"
)

target_include_directories(RasFocus PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(RasFocus PRIVATE
    imgui_lib
    # GDI (CreateSolidBrush, CreateFontW, SelectObject, DrawTextW…)
    gdi32
    # Shell & filesystem
    shell32
    shlwapi
    # COM / OLE (CoCreateInstance, CoInitializeEx, SysAllocString…)
    ole32
    oleaut32
    # Process & memory info (psapi.h)
    psapi
    # UIAutomation COM interface
    uiautomationcore
    # Standard Win32 – user32/kernel32 are always linked by MSVC
)

# ── Compiler flags ─────────────────────────────────────────────
if(MSVC)
    target_compile_options(RasFocus PRIVATE
        /W3         # Warning level 3
        /WX-        # Warnings are NOT errors
        /utf-8      # UTF-8 source (Bengali L"…" literals)
        /MP         # Parallel compilation
    )
    target_compile_definitions(RasFocus PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()
