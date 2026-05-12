add_library(imguizmo STATIC)

target_sources(
    imguizmo
    PRIVATE
        "extern/imguizmo/src/ImGuizmo.cpp"
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "extern/imguizmo/src"
        FILES
            "extern/imguizmo/src/ImGuizmo.h"
)

target_include_directories(imguizmo PRIVATE "${HILL_IMGUI_ROOT}")

set_target_cpp_mode(imguizmo)

if(HILL_DISTRIBUTION)
    target_compile_definitions(imguizmo PRIVATE "NDEBUG")
endif()
