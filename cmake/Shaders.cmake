# HLSL -> backend shader binaries via SDL_shadercross.
#
# Source files are named <name>.<stage>.hlsl where <stage> is vert, frag, or
# comp. Each produces, in ENGINE_SHADER_OUTPUT_DIR:
#   <name>.<stage>.json   resource counts read by the engine's shader loader
#   <name>.<stage>.spv    Vulkan
#   <name>.<stage>.dxil   Direct3D 12   (Windows, or ENGINE_SHADERS_ALL_FORMATS)
#   <name>.<stage>.msl    Metal         (Apple,   or ENGINE_SHADERS_ALL_FORMATS)

option(ENGINE_SHADERS_ALL_FORMATS "Compile shaders for every SDL_GPU backend, not just the host's" OFF)

set(ENGINE_SHADER_FORMATS json spv)
if(WIN32 OR ENGINE_SHADERS_ALL_FORMATS)
    list(APPEND ENGINE_SHADER_FORMATS dxil)
endif()
if(APPLE OR ENGINE_SHADERS_ALL_FORMATS)
    list(APPEND ENGINE_SHADER_FORMATS msl)
endif()

# engine_add_shaders(<target> SOURCES <file.hlsl>... [INCLUDE_DIRS <dir>...])
# Creates a custom target that compiles the given HLSL sources.
function(engine_add_shaders target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "SOURCES;INCLUDE_DIRS")

    set(include_args)
    foreach(dir IN LISTS arg_INCLUDE_DIRS)
        list(APPEND include_args -I "${dir}")
    endforeach()

    set(outputs)
    foreach(source IN LISTS arg_SOURCES)
        cmake_path(ABSOLUTE_PATH source NORMALIZE)
        cmake_path(GET source FILENAME filename)
        string(REGEX MATCH "^(.+)\\.(vert|frag|comp)\\.hlsl$" _ "${filename}")
        if(NOT CMAKE_MATCH_0)
            message(FATAL_ERROR "Shader '${filename}' must be named <name>.<vert|frag|comp>.hlsl")
        endif()
        set(base "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
        set(stage_vert vertex)
        set(stage_frag fragment)
        set(stage_comp compute)
        set(stage "${stage_${CMAKE_MATCH_2}}")

        foreach(format IN LISTS ENGINE_SHADER_FORMATS)
            set(output "${ENGINE_SHADER_OUTPUT_DIR}/${base}.${format}")
            add_custom_command(
                OUTPUT "${output}"
                COMMAND "${SHADERCROSS_EXECUTABLE}" "${source}"
                    --source HLSL --stage ${stage} --entrypoint main
                    ${include_args}
                    --output "${output}"
                DEPENDS "${source}" "${SHADERCROSS_EXECUTABLE}"
                COMMENT "shadercross ${filename} -> ${base}.${format}"
                VERBATIM
            )
            list(APPEND outputs "${output}")
        endforeach()
    endforeach()

    file(MAKE_DIRECTORY "${ENGINE_SHADER_OUTPUT_DIR}")
    add_custom_target(${target} ALL DEPENDS ${outputs} SOURCES ${arg_SOURCES})
endfunction()
