cmake_minimum_required(VERSION 3.20)

include(cmake/sdl.cmake)
include(cmake/imgui.cmake)

add_executable(
    test_sdl
    "src/main.cpp"
    "src/sdl_example.cpp"
    "src/sdl_example.hpp"
)

target_link_libraries(test_sdl PRIVATE hill SDL3::SDL3 imgui_sdl)

target_include_directories(test_sdl PRIVATE "src")

set_target_cpp_mode(test_sdl)
set_target_warnings(test_sdl)
set_target_platform_macros(test_sdl)
