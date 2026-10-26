# Superbuild for the SDL_shadercross command-line tool.
#
# SDL_shadercross publishes no binary releases, so this project builds the
# `shadercross` CLI from pinned sources:
#   SDL3 (shared)  +  SPIRV-Cross (shared C API)  +  prebuilt DXC binaries
#
# It is normally driven automatically by cmake/Shadercross.cmake during the
# first configure of the engine, but it can be run by hand:
#
#   cmake -S tools/shadercross -B build/_shadercross -G Ninja \
#         -DCMAKE_INSTALL_PREFIX=$PWD/external/shadercross
#   cmake --build build/_shadercross
#
# The result is <prefix>/bin/shadercross with its runtime libraries next to it
# (Windows) or in <prefix>/lib (Linux, found via $ORIGIN rpath).
#
# Supported hosts: Linux x86_64 and Windows x64 (DXC ships binaries for
# those). On other hosts, install shadercross yourself and pass
# -DSHADERCROSS_EXECUTABLE=/path/to/shadercross to the engine configure.

cmake_minimum_required(VERSION 3.25)
project(shadercross_superbuild NONE)

include(ExternalProject)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(FATAL_ERROR "Set CMAKE_INSTALL_PREFIX to the shadercross tool directory.")
endif()

set(SDL_URL "https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.18.tar.gz")
set(SDL_SHA256 c4b08b950bd29d83caae0ab8d884298d27046a9d30700fb42841c50992665c90)

set(SPIRV_CROSS_URL "https://github.com/KhronosGroup/SPIRV-Cross/archive/refs/tags/vulkan-sdk-1.4.363.0.tar.gz")
set(SPIRV_CROSS_SHA256 f708982e88b763ef5b0394ed468fc2c3628f68cdab89f2e7a946cd628c04e721)

set(SHADERCROSS_COMMIT 1ff05bec573988a98ef9e0260b4da44f512b8367)
set(SHADERCROSS_URL "https://github.com/libsdl-org/SDL_shadercross/archive/${SHADERCROSS_COMMIT}.tar.gz")
set(SHADERCROSS_SHA256 60aae35ff71bd1d5f85b363e9d077a86bb27dd36333c42685e56c2fd541174d2)

if(CMAKE_HOST_WIN32)
    set(DXC_URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2609/dxc_2026_09_29.zip")
    set(DXC_SHA256 ad31b1fc8443175d204f77a611fdb3ef2ec42759bdc2f1167368de24a4a7e7f1)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(DXC_URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2609/linux_dxc_2026_09_28.x86_x64.tar.gz")
    set(DXC_SHA256 96faadc7f5c282d2ffda49804beb4c3ee38127bc252b723234e3c5cdf7aa39a1)
else()
    message(FATAL_ERROR
        "No prebuilt DXC for ${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}. "
        "Build SDL_shadercross with -DSDLSHADERCROSS_VENDORED=ON and pass "
        "-DSHADERCROSS_EXECUTABLE=<path> to the engine instead.")
endif()

set(prefix "${CMAKE_INSTALL_PREFIX}")
set(common_args
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_PREFIX=${prefix}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_PREFIX_PATH=${prefix}
)
if(CMAKE_GENERATOR)
    list(APPEND common_args -G ${CMAKE_GENERATOR})
endif()

ExternalProject_Add(sdl3
    URL ${SDL_URL}
    URL_HASH SHA256=${SDL_SHA256}
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    CMAKE_ARGS ${common_args}
        -DSDL_SHARED=ON
        -DSDL_STATIC=OFF
        -DSDL_TEST_LIBRARY=OFF
        -DSDL_TESTS=OFF
        -DSDL_EXAMPLES=OFF
)

ExternalProject_Add(spirv_cross
    URL ${SPIRV_CROSS_URL}
    URL_HASH SHA256=${SPIRV_CROSS_SHA256}
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    CMAKE_ARGS ${common_args}
        -DSPIRV_CROSS_SHARED=ON
        -DSPIRV_CROSS_STATIC=OFF
        -DSPIRV_CROSS_CLI=OFF
        -DSPIRV_CROSS_ENABLE_TESTS=OFF
)

# Prebuilt DXC: download and unpack only.
ExternalProject_Add(dxc
    URL ${DXC_URL}
    URL_HASH SHA256=${DXC_SHA256}
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
ExternalProject_Get_Property(dxc SOURCE_DIR)
set(dxc_dir "${SOURCE_DIR}")

# Point FindDirectXShaderCompiler.cmake straight at the unpacked binaries.
if(CMAKE_HOST_WIN32)
    set(dxc_args
        -DDirectXShaderCompiler_INCLUDE_PATH=${dxc_dir}/inc
        -DDirectXShaderCompiler_dxcompiler_BINARY=${dxc_dir}/bin/x64/dxcompiler.dll
        -DDirectXShaderCompiler_dxcompiler_LIBRARY=${dxc_dir}/lib/x64/dxcompiler.lib
        -DDirectXShaderCompiler_dxil_BINARY=${dxc_dir}/bin/x64/dxil.dll
    )
else()
    set(dxc_args
        -DDirectXShaderCompiler_INCLUDE_PATH=${dxc_dir}/include/dxc
        -DDirectXShaderCompiler_dxcompiler_LIBRARY=${dxc_dir}/lib/libdxcompiler.so
        -DDirectXShaderCompiler_dxil_LIBRARY=${dxc_dir}/lib/libdxil.so
    )
endif()

ExternalProject_Add(shadercross
    URL ${SHADERCROSS_URL}
    URL_HASH SHA256=${SHADERCROSS_SHA256}
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    DEPENDS sdl3 spirv_cross dxc
    CMAKE_ARGS ${common_args} ${dxc_args}
        -DSDLSHADERCROSS_VENDORED=OFF
        -DSDLSHADERCROSS_DXC=ON
        -DSDLSHADERCROSS_SHARED=ON
        -DSDLSHADERCROSS_STATIC=OFF
        -DSDLSHADERCROSS_SPIRVCROSS_SHARED=ON
        -DSDLSHADERCROSS_CLI=ON
        -DSDLSHADERCROSS_INSTALL=ON
        # INSTALL_RUNTIME would also build vkd3d (needs bison/autotools);
        # the DXC runtime is copied in by the step below instead.
        -DSDLSHADERCROSS_INSTALL_RUNTIME=OFF
        -DSDLSHADERCROSS_INSTALL_MAN=OFF
        -DSDLSHADERCROSS_INSTALL_CPACK=OFF
    # Cache args bypass the shell, so $ORIGIN reaches the linker verbatim.
    CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_RPATH:STRING=$ORIGIN/../lib
)

# Place the DXC shared libraries where the installed CLI can load them.
if(CMAKE_HOST_WIN32)
    ExternalProject_Add_Step(shadercross install_dxc_runtime
        DEPENDEES install
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${dxc_dir}/bin/x64/dxcompiler.dll ${dxc_dir}/bin/x64/dxil.dll ${prefix}/bin
    )
else()
    ExternalProject_Add_Step(shadercross install_dxc_runtime
        DEPENDEES install
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${dxc_dir}/lib/libdxcompiler.so ${dxc_dir}/lib/libdxil.so ${prefix}/lib
    )
endif()
