function(set_target_cpp_mode target)
    target_compile_features(${target} PRIVATE cxx_std_23)
    set_target_properties(
        ${target} PROPERTIES
        CXX_EXTENSIONS OFF
        CMAKE_CXX_STANDARD_REQUIRED ON
    )
endfunction()

function(set_target_warnings target)
    if(UNIX)
        target_compile_options(${target} PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wconversion")
    elseif(WIN32)
        target_compile_options(${target} PRIVATE "/W4")
        target_compile_definitions(${target} PRIVATE "_CRT_SECURE_NO_WARNINGS")
    endif()
endfunction()

function(set_target_platform_macros target)
    if(UNIX)
        target_compile_definitions(${target} PRIVATE "HILL_LINUX")
    elseif(WIN32)
        target_compile_definitions(${target} PRIVATE "HILL_WINDOWS")
    endif()
endfunction()
