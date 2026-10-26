# Third-party libraries, fetched at pinned versions with FetchContent.
#
# Day-one dependencies are always enabled. Everything else in the technology
# stack is opt-in so it is only integrated once a milestone needs it:
#
#   cmake --preset debug -DENGINE_WITH_ENTT=ON -DENGINE_WITH_IMGUI=ON
#
# Each optional library is exposed to engine code through the
# engine_thirdparty interface target together with an ENGINE_HAS_<NAME>
# compile definition.

include(FetchContent)

option(ENGINE_WITH_ENTT      "ECS: EnTT"                      OFF)
option(ENGINE_WITH_BOX2D     "2D physics: Box2D"              OFF)
option(ENGINE_WITH_JOLT      "3D physics: Jolt Physics"       OFF)
option(ENGINE_WITH_MINIAUDIO "Audio: miniaudio"               OFF)
option(ENGINE_WITH_IMGUI     "Debug UI: Dear ImGui"           OFF)
option(ENGINE_WITH_STB       "Image loading: stb_image"       OFF)
option(ENGINE_WITH_TINYGLTF  "Model loading: tinygltf"        OFF)
option(ENGINE_WITH_JSON      "Serialization: nlohmann/json"   OFF)
option(ENGINE_WITH_SPDLOG    "Logging: spdlog"                OFF)
option(ENGINE_WITH_TRACY     "Profiling: Tracy client"        OFF)

add_library(engine_thirdparty INTERFACE)
add_library(engine::thirdparty ALIAS engine_thirdparty)

# Dependencies are third-party code: don't let our warning flags or
# clang-tidy loose on them, and keep their tests/examples out of the build.
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

# --- Day one ---------------------------------------------------------------

# SDL3: platform, window, input, audio, and SDL_GPU.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SDL3
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.18.tar.gz
    URL_HASH SHA256=c4b08b950bd29d83caae0ab8d884298d27046a9d30700fb42841c50992665c90
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    SYSTEM
)

# GLM: vectors, matrices, quaternions. SDL_GPU clip space has depth in [0, 1].
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glm
    URL https://github.com/g-truc/glm/archive/refs/tags/1.0.3.tar.gz
    URL_HASH SHA256=6775e47231a446fd086d660ecc18bcd076531cfedd912fbd66e576b118607001
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    SYSTEM
)

FetchContent_MakeAvailable(SDL3 glm)
target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_RADIANS)

if(ENGINE_BUILD_TESTS)
    FetchContent_Declare(Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.16.1.tar.gz
        URL_HASH SHA256=d16bd1b8b2364918bd472cb216394acb93bde6784890df0d91bccfb3f28d777d
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        SYSTEM
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

# --- Optional, introduced per milestone --------------------------------------

if(ENGINE_WITH_ENTT)
    FetchContent_Declare(EnTT
        URL https://github.com/skypjack/entt/archive/refs/tags/v4.0.0.tar.gz
        URL_HASH SHA256=32a2ff2c72cb047dfd57306006ef238820b70da7c6ce4e7e8a507ac63365212e
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        SYSTEM
    )
    FetchContent_MakeAvailable(EnTT)
    target_link_libraries(engine_thirdparty INTERFACE EnTT::EnTT)
    target_compile_definitions(engine_thirdparty INTERFACE ENGINE_HAS_ENTT)
endif()

if(ENGINE_WITH_BOX2D)
    set(BOX2D_SAMPLES OFF CACHE BOOL "" FORCE)
    set(BOX2D_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    set(BOX2D_BENCHMARKS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(box2d
        URL https://github.com/erincatto/box2d/archive/refs/tags/v3.2.0.tar.gz
        URL_HASH SHA256=ffec747b27a2267016f8d0240d658c82d187fb2271fa0f69fcbe53fbc067180d
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        SYSTEM
    )
    FetchContent_MakeAvailable(box2d)
    target_link_libraries(engine_thirdparty INTERFACE box2d::box2d)
    target_compile_definitions(engine_thirdparty INTERFACE ENGINE_HAS_BOX2D)
endif()

if(ENGINE_WITH_JOLT)
    set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
    set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
    set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
    set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)
    set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "" FORCE)
    set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(JoltPhysics
        URL https://github.com/jrouwe/JoltPhysics/archive/refs/tags/v5.6.0.tar.gz
        URL_HASH SHA256=6e069ee0172478cc78182047aac87e5310ba14a67a53348ae14cc37801fd3f8e
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        SOURCE_SUBDIR Build
        SYSTEM
    )
    FetchContent_MakeAvailable(JoltPhysics)
    target_link_libraries(engine_thirdparty INTERFACE Jolt)
    target_compile_definitions(engine_thirdparty INTERFACE ENGINE_HAS_JOLT)
endif()

if(ENGINE_WITH_MINIAUDIO)
    set(MINIAUDIO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(MINIAUDIO_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(miniaudio
        URL https://github.com/mackron/miniaudio/archive/refs/tags/0.11.25.tar.gz
        URL_HASH SHA256=b900edcffe979816e2560a0580b9b1216d674b4f17fbadeca8f777a7f8ab0274
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        SYSTEM
    )
    FetchContent_MakeAvailable(miniaudio)
    target_link_libraries(engine_thirdparty INTERFACE miniaudio)
    target_compile_definitions(engine_thirdparty INTERFACE ENGINE_HAS_MINIAUDIO)
endif()

if(ENGINE_WITH_IMGUI)
    # Dear ImGui ships no CMake project; build the core plus SDL3/SDL_GPU backends.
    FetchContent_Declare(imgui
        URL https://github.com/ocornut/imgui/archive/refs/tags/v1.93.0.tar.gz
        URL_HASH SHA256=f7bf6bc76e5d79d40d751096663e6c5d18daf08129dd52e7f7b09e4251c8eeee
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )
    FetchContent_MakeAvailable(imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp
    )
    target_include_directories(imgui SYSTEM PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
    target_link_libraries(imgui PUBLIC SDL3::SDL3)
    target_link_libraries(engine_thirdparty INTERFACE imgui)
    target_compile_definitions(engine_thirdparty INTERFACE ENGINE_HAS_IMGUI)
endif()

if(ENGINE_WITH_STB)
    # Header-only; define STB_IMAGE_IMPLEMENTATION in exactly one .cpp file.
    FetchContent_Declare(stb
        URL https://github.com/nothings/stb/archive/003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5.tar.gz
        URL_HASH SHA256=a6c31950b3bdf05eb92258768ba2a11bd44449490574b1aac67028452288c40d
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )
    FetchContent_MakeAvailable(stb)
    add_library(stb INTERFACE)
    target_include_directories(stb SYSTEM INTERFACE ${stb_SOURCE_DIR})
    target_link_libraries(engine_thirdparty INTERFACE stb)
    target_compile_definitions(engine_thirdparty INTERFACE ENGINE_HAS_STB)
endif()

if(ENGINE_WITH_TINYGLTF)
    set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(TINYGLTF_BUILD_GL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(TINYGLTF_BUILD_VALIDATOR_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(TINYGLTF_BUILD_BUILDER_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(TINYGLTF_HEADER_ONLY ON CACHE BOOL "" FORCE)
    set(TINYGLTF_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(tinygltf
        URL https://github.com/syoyo/tinygltf/archive/refs/tags/v2.9.7.tar.gz
        URL_HASH SHA256=bf92fe53b86302f9415e77504ecd81a58659292ed04dffec873f5c71665046a0
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        SYSTEM
    )
    FetchContent_MakeAvailable(tinygltf)
    target_link_libraries(engine_thirdparty INTERFACE tinygltf)
    target_compile_definitions(engine_thirdparty INTERFACE ENGINE_HAS_TINYGLTF)
endif()

if(ENGINE_WITH_JSON)
    FetchContent_Declare(nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
        URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        SYSTEM
    )
    FetchContent_MakeAvailable(nlohmann_json)
    target_link_libraries(engine_thirdparty INTERFACE nlohmann_json::nlohmann_json)
    target_compile_definitions(engine_thirdparty INTERFACE ENGINE_HAS_JSON)
endif()

if(ENGINE_WITH_SPDLOG)
    set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "" FORCE)
    FetchContent_Declare(spdlog
        URL https://github.com/gabime/spdlog/archive/refs/tags/v1.17.0.tar.gz
        URL_HASH SHA256=d8862955c6d74e5846b3f580b1605d2428b11d97a410d86e2fb13e857cd3a744
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        SYSTEM
    )
    FetchContent_MakeAvailable(spdlog)
    target_link_libraries(engine_thirdparty INTERFACE spdlog::spdlog)
    target_compile_definitions(engine_thirdparty INTERFACE ENGINE_HAS_SPDLOG)
endif()

if(ENGINE_WITH_TRACY)
    # The client only records when TRACY_ENABLE is on; connect the Tracy
    # profiler GUI (built separately, same version) to view captures.
    set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
    set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
    FetchContent_Declare(tracy
        URL https://github.com/wolfpld/tracy/archive/refs/tags/v0.14.1.tar.gz
        URL_HASH SHA256=bf4af567e9c7524d07f3caa745fad02fb33bd5694f11910750382d1efbb251c1
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        SYSTEM
    )
    FetchContent_MakeAvailable(tracy)
    target_link_libraries(engine_thirdparty INTERFACE Tracy::TracyClient)
    target_compile_definitions(engine_thirdparty INTERFACE ENGINE_HAS_TRACY)
endif()
