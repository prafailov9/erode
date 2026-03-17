cmake_minimum_required(VERSION 3.16)
project(erode)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── GoogleTest ────────────────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ── Find SFML ─────────────────────────────────────────────────────────────────
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

# ── Library (shared code) ─────────────────────────────────────────────────────
add_library(erode_lib
    src/Grid.cpp
    src/Renderer.cpp
    src/InputHandler.cpp
    src/Panel.cpp
    src/velocity/Velocity.cpp
)

target_include_directories(erode_lib PUBLIC src)

target_link_libraries(erode_lib PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(erode src/main.cpp)

target_link_libraries(erode PRIVATE
    erode_lib
)

# ── Tests ─────────────────────────────────────────────────────────────────────
add_executable(erode_tests
    tests/GridTest.cpp
)

target_link_libraries(erode_tests PRIVATE
    erode_lib
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(erode_tests)