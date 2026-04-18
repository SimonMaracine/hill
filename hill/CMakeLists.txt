cmake_minimum_required(VERSION 3.20)

add_library(hill STATIC)

target_sources(
    hill
    PRIVATE
        "src/hill.cpp"
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "include"
        FILES
            "include/hill/hill.hpp"
)

set_target_cpp_mode(hill)
set_target_warnings(hill)
set_target_platform_macros(hill)

if(HILL_DISTRIBUTION)
    target_compile_definitions(hill PRIVATE "NDEBUG")
endif()
