if(NOT HILL_DISTRIBUTION)
    set(IMGUI_DEMO "extern/imgui/imgui_demo.cpp")
endif()

add_library(imgui STATIC)

target_sources(
    imgui
    PRIVATE
        ${IMGUI_DEMO}
        "extern/imgui/imgui_draw.cpp"
        "extern/imgui/imgui_tables.cpp"
        "extern/imgui/imgui_widgets.cpp"
        "extern/imgui/imgui.cpp"
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "extern/imgui"
        FILES
            "extern/imgui/imconfig.h"
            "extern/imgui/imgui_internal.h"
            "extern/imgui/imgui.h"
            "extern/imgui/imstb_rectpack.h"
            "extern/imgui/imstb_textedit.h"
            "extern/imgui/imstb_truetype.h"
)

set_target_cpp_mode(imgui)

if(HILL_DISTRIBUTION)
    target_compile_definitions(imgui PRIVATE "NDEBUG")
endif()

set(HILL_IMGUI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui")
