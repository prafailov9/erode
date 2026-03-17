cmake_minimum_required(VERSION 3.16)
project(erode)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Source files ──────────────────────────────────────────────────────────────
add_executable(erode
    src/main.cpp
    src/Grid.cpp
    src/Renderer.cpp
    src/InputHandler.cpp
    src/Panel.cpp
    src/velocity/Velocity.cpp
)

# ── Find SFML ─────────────────────────────────────────────────────────────────
# Set SFML_DIR (or CMAKE_PREFIX_PATH) to your SFML 3 install, e.g.:
#   cmake -DSFML_DIR="C:/SFML-3/lib/cmake/SFML" ..
# or install via vcpkg and use its toolchain file.
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

target_link_libraries(erode PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

# ── Include dirs ──────────────────────────────────────────────────────────────
target_include_directories(erode PRIVATE src)

# ── Optional: copy assets next to the binary ─────────────────────────────────
# Uncomment when you add an assets/ folder
# add_custom_command(TARGET erode POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:erode>/assets
# )