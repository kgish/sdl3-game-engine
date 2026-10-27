cmake_minimum_required(VERSION 3.28)

project(sdl3-game-engine
    VERSION 0.1.0
    DESCRIPTION "Reusable C++23 game engine on SDL3 and SDL_GPU"
    LANGUAGES C CXX
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported. Use a preset: cmake --preset debug")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Executables and compiled shaders land side by side so SDL_GetBasePath()
# finds the shaders at runtime.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(ENGINE_SHADER_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)

option(ENGINE_BUILD_GAMES "Build the sample games" ON)
option(ENGINE_BUILD_TESTS "Build the unit tests" ON)
option(ENGINE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ENGINE_ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(ENGINE_ENABLE_CLANG_TIDY "Run clang-tidy while compiling engine code" OFF)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(CompilerOptions)
include(Dependencies)
include(Shadercross)
include(Shaders)

add_subdirectory(shaders)
add_subdirectory(engine)

if(ENGINE_BUILD_GAMES)
    add_subdirectory(games)
endif()

if(ENGINE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
