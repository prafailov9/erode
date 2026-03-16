cmake_minimum_required(VERSION 3.16)
project(erode)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Set module path for finding dependencies ──────────────────────────────────
list(APPEND CMAKE_PREFIX_PATH /usr /usr/local /opt/local)

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
# Looks for SFML installed via your package manager or a local install.
# Requires: Graphics, Window, System components.
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

target_link_libraries(erode
    sfml-graphics
    sfml-window
    sfml-system
)

# ── Include dirs ──────────────────────────────────────────────────────────────
target_include_directories(erode PRIVATE src)

# ── Optional: copy assets next to the binary ─────────────────────────────────
# Uncomment when you add an assets/ folder
# add_custom_command(TARGET erode POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:erode>/assets
# )