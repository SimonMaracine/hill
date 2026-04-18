cmake_minimum_required(VERSION 3.20)

project(
    "hill-render-engine"
    VERSION "0.1.0"
    DESCRIPTION "Platform and windowing system agnostic 3D graphics rendering engine"
    HOMEPAGE_URL "https://github.com/SimonMaracine/hill"
    LANGUAGES CXX C
)

include(cmake/options.cmake)

set(CACHE{CMAKE_BUILD_TYPE} TYPE STRING VALUE "Debug")
set(CACHE{CMAKE_CONFIGURATION_TYPES} TYPE STRING FORCE VALUE "Debug;Release")
set(BUILD_SHARED_LIBS OFF)

include(cmake/sanity_checks.cmake)
include(cmake/target_options.cmake)

add_subdirectory(hill)

if(HILL_BUILD_TESTS)
    add_subdirectory(tests)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

message(STATUS "HILL_DISTRIBUTION: ${HILL_DISTRIBUTION}")
message(STATUS)
