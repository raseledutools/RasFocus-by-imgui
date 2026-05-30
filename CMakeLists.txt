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

# ── Minimal Cross-Platform stub ────────────────────────────────
set(STUB_MAIN "${CMAKE_CURRENT_BINARY_DIR}/stub_main.cpp")
if(WIN32)
    # Windows entry point — also defines the extern globals from tab_adult.h
    file(WRITE "${STUB_MAIN}" [[
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return 0; }
]])
else()
    # Standard entry point for Linux/Mac — also defines the extern globals from tab_adult.h
    file(WRITE "${STUB_MAIN}" [[
int main(int argc, char** argv) { return 0; }
]])
endif()

# ── RasFocus executable ────────────────────────────────────────
if(WIN32)
    add_executable(RasFocus WIN32 tab_adult.cpp "${STUB_MAIN}")
elseif(APPLE)
    add_executable(RasFocus MACOSX_BUNDLE tab_adult.cpp "${STUB_MAIN}")
else()
    add_executable(RasFocus tab_adult.cpp "${STUB_MAIN}")
endif()

target_include_directories(RasFocus PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# ── Platform Specific Libraries & Flags ────────────────────────
if(WIN32)
    target_link_libraries(RasFocus PRIVATE
        imgui_lib
        gdi32 shell32 shlwapi ole32 oleaut32 psapi uiautomationcore
    )
    if(MSVC)
        target_compile_options(RasFocus PRIVATE
            /W3 /WX- /utf-8 /MP
        )
        target_compile_definitions(RasFocus PRIVATE
            UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
        )
    endif()
    
elseif(APPLE)
    # macOS specific frameworks
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(RasFocus PRIVATE 
        imgui_lib 
        ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY}
    )
    
else()
    # Linux specific libraries
    find_package(OpenGL REQUIRED)
    find_package(Threads REQUIRED)
    target_link_libraries(RasFocus PRIVATE 
        imgui_lib 
        OpenGL::GL Threads::Threads dl X11
    )
endif()