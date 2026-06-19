cmake_minimum_required(VERSION 3.16)
project(CS2_RCON_Panel VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies ──────────────────────────────────────────────────────────────

# GLFW — local source (downloaded from codeload.github.com)
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw")
set(GLFW_BUILD_DOCS   OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS  OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${GLFW_DIR} ${CMAKE_BINARY_DIR}/glfw EXCLUDE_FROM_ALL)

# Dear ImGui — local sources (downloaded from CDN)
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/rcon_client.cpp
    src/theme.cpp
    src/ui.cpp
    ${IMGUI_SOURCES}
)

add_executable(cs2_rcon_panel WIN32 ${SOURCES})

target_include_directories(cs2_rcon_panel PRIVATE
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${GLFW_DIR}/include
)

target_link_libraries(cs2_rcon_panel PRIVATE
    glfw
)

if(WIN32)
    target_link_libraries(cs2_rcon_panel PRIVATE opengl32 ws2_32)
endif()

# ── Compiler flags ────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_definitions(cs2_rcon_panel PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _WINSOCK_DEPRECATED_NO_WARNINGS
        IMGUI_USE_WCHAR32
    )
    target_compile_options(cs2_rcon_panel PRIVATE /W4 /utf-8)
    # /SUBSYSTEM:WINDOWS is already set by add_executable(WIN32 ...)
    # We provide a WinMain() entry point wrapper in main.cpp
endif()
