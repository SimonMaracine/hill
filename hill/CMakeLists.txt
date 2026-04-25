cmake_minimum_required(VERSION 3.20)

include(cmake/glad.cmake)
include(cmake/glm.cmake)
include(cmake/imgui.cmake)
include(cmake/stb_image.cmake)

add_library(hill STATIC)

target_sources(
    hill
    PRIVATE
        "src/primitives/common.cpp"
        "src/primitives/element_buffer.cpp"
        "src/primitives/shader.cpp"
        "src/primitives/texture2d.cpp"
        "src/primitives/vertex_array.cpp"
        "src/primitives/vertex_buffer.cpp"
        "src/debug.cpp"
        "src/editor.cpp"
        "src/glad.cpp"
        "src/graphics_api.cpp"
        "src/image.cpp"
        "src/mesh.cpp"
        "src/primitives_registry.cpp"
        "src/renderer.cpp"
        "src/renderer_command.cpp"
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "include"
        FILES
            "include/hill/primitives/common.hpp"
            "include/hill/primitives/element_buffer.hpp"
            "include/hill/primitives/shader.hpp"
            "include/hill/primitives/texture2d.hpp"
            "include/hill/primitives/vertex_array.hpp"
            "include/hill/primitives/vertex_buffer.hpp"
            "include/hill/camera.hpp"
            "include/hill/configuration.hpp"
            "include/hill/debug.hpp"
            "include/hill/editor.hpp"
            "include/hill/error.hpp"
            "include/hill/graphics_api.hpp"
            "include/hill/image.hpp"
            "include/hill/imgui.hpp"
            "include/hill/mesh.hpp"
            "include/hill/primitives_registry.hpp"
            "include/hill/renderer.hpp"
            "include/hill/renderer_command.hpp"
)

target_link_libraries(hill PRIVATE glad)
target_link_libraries(hill PUBLIC imgui glm::glm stb_image)

set_target_cpp_mode(hill)
set_target_warnings(hill)
set_target_platform_macros(hill)

if(HILL_DISTRIBUTION)
    target_compile_definitions(hill PRIVATE "NDEBUG")
endif()

set(HILL_IMGUI_ROOT "${HILL_IMGUI_ROOT}" PARENT_SCOPE)
