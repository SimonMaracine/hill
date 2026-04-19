add_library(imgui_sdl STATIC)

message(STATUS "${HILL_IMGUI_ROOT}")
message(STATUS "${HILL_SDL_INCLUDE_DIRECTORY}")

target_sources(
    imgui_sdl
    PRIVATE
        "${HILL_IMGUI_ROOT}/backends/imgui_impl_sdl3.cpp"
        "${HILL_IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp"
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "${HILL_IMGUI_ROOT}"
        FILES
            "${HILL_IMGUI_ROOT}/backends/imgui_impl_sdl3.h"
            "${HILL_IMGUI_ROOT}/backends/imgui_impl_opengl3.h"
)

target_include_directories(imgui_sdl PRIVATE "${HILL_SDL_INCLUDE_DIRECTORY}")

set_target_cpp_mode(imgui_sdl)

if(HILL_DISTRIBUTION)
    target_compile_definitions(imgui_sdl PRIVATE "NDEBUG")
endif()
