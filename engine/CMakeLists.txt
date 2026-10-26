add_library(engine STATIC
    src/core/Application.cpp
    src/core/Clock.cpp
    src/core/Logger.cpp
    src/platform/sdl3/SDLInput.cpp
    src/platform/sdl3/SDLWindow.cpp
    src/renderer/Camera.cpp
    src/renderer/Renderer2D.cpp
    src/renderer/gpu/GPUShader.cpp
    src/renderer/gpu/SDLGPURenderDevice.cpp
)
add_library(engine::engine ALIAS engine)

target_include_directories(engine
    PUBLIC include
    PRIVATE src
)

# glm appears in public headers; SDL3 stays an implementation detail.
target_link_libraries(engine
    PUBLIC glm::glm engine::thirdparty
    PRIVATE SDL3::SDL3
)

target_compile_features(engine PUBLIC cxx_std_23)
engine_target_defaults(engine)

# Every engine consumer needs the compiled shaders at runtime.
add_dependencies(engine engine_shaders)
