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

# ── ImGui static library (core + Win32/DX11 backends) ─────────
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# ── RasFocus sources ───────────────────────────────────────────
set(RASFOCUS_SOURCES
    main.cpp
    tab_adult.cpp
    tab_settings.cpp
    tab_schedule.cpp
    tab_family_link.cpp
)

add_executable(RasFocus WIN32 ${RASFOCUS_SOURCES})

target_include_directories(RasFocus PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# ── Link libraries ─────────────────────────────────────────────
target_link_libraries(RasFocus PRIVATE
    imgui_lib
    d3d11
    dxgi
    gdi32
    shell32
    shlwapi
    ole32
    oleaut32
    psapi
    uiautomationcore
)

# ── MSVC flags ─────────────────────────────────────────────────
if(MSVC)
    target_compile_options(RasFocus PRIVATE
        /W3 /WX- /utf-8 /MP
    )
    target_compile_definitions(RasFocus PRIVATE
        UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
    )
endif()
