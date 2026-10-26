# Locates the SDL_shadercross CLI, building it on first configure if needed.
#
# Resolution order:
#   1. -DSHADERCROSS_EXECUTABLE=<path>            (explicit override)
#   2. shadercross on PATH or in external/shadercross/bin
#   3. Bootstrap: build tools/shadercross into external/shadercross
#      (one-off, a few minutes; shared by all build presets)

set(ENGINE_SHADERCROSS_PREFIX "${PROJECT_SOURCE_DIR}/external/shadercross"
    CACHE PATH "Install location of the bootstrapped shadercross tool")
option(ENGINE_BOOTSTRAP_SHADERCROSS "Build SDL_shadercross from source when it is not found" ON)

find_program(SHADERCROSS_EXECUTABLE shadercross
    HINTS "${ENGINE_SHADERCROSS_PREFIX}/bin"
)

if(NOT SHADERCROSS_EXECUTABLE AND ENGINE_BOOTSTRAP_SHADERCROSS)
    set(_sc_build_dir "${PROJECT_SOURCE_DIR}/external/_build/shadercross")
    message(STATUS "shadercross not found; building it into ${ENGINE_SHADERCROSS_PREFIX} (one-off)")

    set(_sc_generator_args)
    if(CMAKE_GENERATOR)
        list(APPEND _sc_generator_args -G "${CMAKE_GENERATOR}")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -S "${PROJECT_SOURCE_DIR}/tools/shadercross"
            -B "${_sc_build_dir}"
            ${_sc_generator_args}
            "-DCMAKE_INSTALL_PREFIX=${ENGINE_SHADERCROSS_PREFIX}"
        RESULT_VARIABLE _sc_result
    )
    if(_sc_result EQUAL 0)
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${_sc_build_dir}"
            RESULT_VARIABLE _sc_result
        )
    endif()
    if(NOT _sc_result EQUAL 0)
        message(FATAL_ERROR "Building shadercross failed. See output above, or pass -DSHADERCROSS_EXECUTABLE=<path>.")
    endif()

    # The installed tool is self-contained; drop the ~200 MB build tree.
    file(REMOVE_RECURSE "${_sc_build_dir}")

    unset(SHADERCROSS_EXECUTABLE CACHE)
    find_program(SHADERCROSS_EXECUTABLE shadercross
        HINTS "${ENGINE_SHADERCROSS_PREFIX}/bin"
        NO_DEFAULT_PATH
        REQUIRED
    )
endif()

if(NOT SHADERCROSS_EXECUTABLE)
    message(FATAL_ERROR "shadercross not found. Enable ENGINE_BOOTSTRAP_SHADERCROSS or set SHADERCROSS_EXECUTABLE.")
endif()

message(STATUS "Using shadercross: ${SHADERCROSS_EXECUTABLE}")
