# SDL3_GAME_ENGINE.md

Building a Reusable C++ Game Engine with SDL3 and SDL_GPU

A consolidated guide from our discussion: library selection, architecture, technology stack, first project, reading recommendations, SDL_GPU trade-offs, and refinements to the interfaces and project layout.

## 1. Objective and recommendation

Build a reusable **C++23** game engine supporting **2D first, then 3D**, with **SDL3** for platform services and **SDL_GPU** for rendering. Use CMake and Ninja, isolate rendering details from gameplay, and validate the architecture with Breakout followed by Asteroids and a simple 3D sandbox.

SDL3 and OpenGL are not direct substitutes: SDL3 provides platform/window/input/audio services, whereas OpenGL is a graphics API. SDL3 can be paired with OpenGL, but SDL_GPU is a compelling modern alternative.

## 2. SDL3 versus OpenGL

| Capability | SDL3 | OpenGL |
|---|---|---|
| Windows, input, controllers, audio | Yes | No |
| Simple 2D drawing | SDL_Renderer | Custom implementation |
| Advanced 2D / 3D | SDL_GPU or another API | Yes |
| Shader control | Via SDL_GPU | Yes |
| Cross-platform | Strong | Strong, with caveats; deprecated on Apple platforms |
| Complexity | Low for SDL_Renderer, higher for SDL_GPU | Medium to high |

**SDL3 advantages:** portable application infrastructure, input and audio, gamepad support, C API that works well with C++, and freedom to choose a renderer.

**SDL3 disadvantages:** no built-in scene graph, general-purpose ECS, physics engine, animation system, or asset manager.

**OpenGL advantages:** flexible GPU rendering, mature educational resources, shader programming, and direct graphics control.

**OpenGL disadvantages:** no input/audio/window lifecycle by itself, considerable resource and state management, and Apple platform deprecation.

A reasonable OpenGL alternative is SDL3 for platform services and OpenGL for graphics.

## 3. Other low-level libraries

| Library | Best fit | Advantages | Trade-offs | Link |
|---|---|---|---|---|
| raylib | Quickly making 2D/3D games | Minimal boilerplate, approachable | Less suitable for experimenting with multiple render backends | https://www.raylib.com/ |
| GLFW | Learning OpenGL | Small window/input/context layer | No audio or game framework | https://www.glfw.org/ |
| SFML | C++-friendly 2D | Convenient graphics/audio APIs | Primarily 2D-oriented | https://www.sfml-dev.org/ |
| bgfx | Multi-backend renderer | Portable advanced rendering | More setup, no full game framework | https://bkaradzic.github.io/bgfx/ |
| sokol | Minimal embedded graphics | Lightweight, readable, portable | Lower-level, fewer game conveniences | https://github.com/floooh/sokol |

For fast game creation choose raylib; for reusable engine development choose SDL3; for graphics-API learning consider GLFW + OpenGL.

## 4. Why SDL_GPU?

SDL3 provides two distinct rendering levels:

| Feature | SDL_Renderer | SDL_GPU |
|---|---|---|
| Sprite/rectangle drawing | Built-in | Implement yourself |
| Custom graphics pipelines | Not its standard model | Yes |
| 3D | Not designed for it | Yes |
| GPU buffers / render passes | Mostly hidden | Explicit |
| Difficulty | Low | Medium–high |

SDL_GPU exposes graphics and compute pipelines, buffers, textures, command buffers, depth/stencil, render passes, and shaders. Its native backends include **Vulkan, Direct3D 12, and Metal**; it is **not an OpenGL backend**.

### Advantages

- One rendering API across multiple modern desktop graphics backends.
- Modern explicit graphics concepts without the full boilerplate of native Vulkan.
- Shared GPU infrastructure for 2D sprites and 3D meshes.
- Custom shaders, offscreen rendering, batching, lighting, post-processing, and compute workloads.
- A suitable foundation for a self-built renderer.

### Disadvantages

- No built-in sprite engine: write quads, blending, batching, cameras, and texture uploads.
- Build and distribute backend-compatible shader binaries; SDL_shadercross is useful.
- Smaller tutorial ecosystem than OpenGL.
- Portable feature set does not expose every advanced native GPU feature (e.g., hardware ray tracing or mesh shaders).
- Platform limitations matter, particularly web and some mobile/older-device targets; verify current SDL support before committing to them.
- GPU resource lifetime and synchronization need care.

### SDL_GPU versus OpenGL versus Vulkan

| Criterion | SDL_GPU | OpenGL | Vulkan |
|---|---|---|---|
| Learning curve | Medium–high | Medium | Very high |
| API model | Modern explicit | Stateful legacy/modern mix | Explicit |
| Native Metal backend | Yes | No | No (requires translation) |
| Educational ecosystem | Smaller | Extensive | Extensive |
| Advanced GPU features | Portable subset | Version-dependent | Extensive |
| Custom engine suitability | Excellent | Very good | Excellent |

Actual performance depends much more on renderer design and workloads than on the API label alone.

Documentation: https://wiki.libsdl.org/SDL3/CategoryGPU

Examples: https://github.com/TheSpydog/SDL_gpu_examples

Shader tools: https://github.com/libsdl-org/SDL_shadercross

## 5. Recommended technology stack

| Subsystem | Choice | Introduce when |
|---|---|---|
| Language | C++23 | Day one |
| Build | CMake + Ninja | Day one |
| Platform / input | SDL3 | Day one |
| GPU rendering | SDL_GPU | Day one |
| Shader pipeline | SDL_shadercross | First triangle |
| Math | GLM | First camera / transforms |
| Unit tests | Catch2 | Day one |
| ECS | EnTT | Once entities become useful |
| 2D physics | Box2D | If simple collision becomes insufficient |
| 3D physics | Jolt Physics | Later 3D projects |
| Audio | miniaudio or SDL3 Audio | First sound effects |
| Debug UI | Dear ImGui | Debugging / tooling milestone |
| Textures / models | stb_image, tinygltf | Asset milestone / 3D milestone |
| Serialization | nlohmann/json | Scenes and configs |
| Logging | spdlog | As logging needs grow |
| Profiling | Tracy | Performance work |
| GPU debugger | RenderDoc | GPU debugging |

Do not integrate every dependency at the start.

## 6. Architecture

```text
Game applications (Breakout / Asteroids / 3D sandbox)
                    |
          Engine public API
   (Application, Game, World, Assets, Input)
                    |
      +-------------+-------------+
      |                           |
   Renderer2D                  Renderer3D
      |                           |
      +-------------+-------------+
                    |
                RenderDevice
          (resources and commands)
                    |
           SDL_GPU implementation
                    |
       Vulkan / Direct3D 12 / Metal

Other engine systems:
  Core lifecycle / fixed timestep / ECS / assets / audio / physics
Platform services:
  SDL3 window / input / OS integration
```

Principles:

1. Engine and games are separate CMake targets.
2. Gameplay code never calls SDL_GPU directly.
3. 2D and 3D share asset, input, ECS, and GPU-resource infrastructure.
4. Keep physics optional and separated.
5. Dependencies point toward stable engine APIs.
6. Build only abstractions that serve current needs.
7. Start with one backend implementation, even if an interface permits more.

## 7. Updated directory structure

```text
my-engine/
├── CMakeLists.txt
├── CMakePresets.json
├── cmake/
│   ├── Dependencies.cmake
│   └── Shaders.cmake
├── external/
├── engine/
│   ├── CMakeLists.txt
│   ├── include/engine/
│   │   ├── Application.hpp
│   │   ├── Game.hpp
│   │   ├── World.hpp
│   │   ├── Renderer.hpp
│   │   ├── Assets.hpp
│   │   └── Types.hpp
│   └── src/
│       ├── core/
│       │   ├── Application.cpp
│       │   ├── Clock.cpp
│       │   └── Logger.cpp
│       ├── platform/sdl3/
│       │   ├── SDLWindow.cpp
│       │   └── SDLInput.cpp
│       ├── renderer/
│       │   ├── RenderDevice.hpp
│       │   ├── Renderer2D.cpp
│       │   ├── Renderer3D.cpp
│       │   ├── Camera.cpp
│       │   └── gpu/
│       │       ├── SDLGPURenderDevice.cpp
│       │       ├── GPUBuffer.cpp
│       │       ├── GPUTexture.cpp
│       │       ├── GPUPipeline.cpp
│       │       └── GPUShader.cpp
│       ├── world/
│       ├── physics/
│       ├── audio/
│       └── assets/
├── games/
│   ├── breakout/
│   ├── asteroids/
│   └── sandbox3d/
├── shaders/
│   ├── source/
│   │   ├── sprite.vert.hlsl
│   │   ├── sprite.frag.hlsl
│   │   ├── mesh.vert.hlsl
│   │   └── mesh.frag.hlsl
│   └── compiled/           # Generated output
├── tools/editor/
└── tests/
```

Build the engine initially as a static library; avoid unnecessary shared-library ABI complexity. Keep generated shaders out of Git unless distribution requirements justify them.

## 8. Core C++ interfaces

### Game and Application

```cpp
#pragma once
#include <memory>

namespace engine {

class Renderer2D;
class World;
class Input;
class Assets;

struct GameContext {
    Renderer2D& renderer2D;
    World& world;
    Input& input;
    Assets& assets;
};

class Game {
public:
    virtual ~Game() = default;
    virtual void onInit(GameContext& ctx) {}
    virtual void onFixedUpdate(GameContext& ctx, double dt) {}
    virtual void onUpdate(GameContext& ctx, double dt) {}
    virtual void onRender(GameContext& ctx) {}
    virtual void onShutdown(GameContext& ctx) {}
};

struct AppConfig {
    const char* title = "My Engine";
    int width = 1280;
    int height = 720;
    bool vsync = true;
};

class Application {
public:
    explicit Application(AppConfig config);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run(Game& game);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace engine
```

The application owns engine services, which must outlive `GameContext` references. Add `Renderer3D` to the context when needed. The destructor for the PImpl class belongs in the `.cpp` file.

### High-level Renderer2D

```cpp
class Renderer2D {
public:
    void beginScene(const Camera2D& camera);

    void drawSprite(
        TextureHandle texture,
        const glm::mat4& transform,
        const glm::vec4& tint
    );

    void drawRect(
        const glm::mat4& transform,
        const glm::vec4& color
    );

    void endScene();
};
```

### Low-level RenderDevice

```cpp
class RenderDevice {
public:
    virtual ~RenderDevice() = default;

    virtual BufferHandle createBuffer(const BufferDesc& desc) = 0;
    virtual TextureHandle createTexture(const TextureDesc& desc) = 0;
    virtual PipelineHandle createPipeline(const PipelineDesc& desc) = 0;

    virtual void destroyBuffer(BufferHandle handle) = 0;
    virtual void destroyTexture(TextureHandle handle) = 0;
    virtual void destroyPipeline(PipelineHandle handle) = 0;
};
```

These are **design sketches**, not complete compilable files: `TextureHandle`, `BufferDesc`, `Camera2D`, and related types need definitions. The eventual render device also needs frame acquisition, uploads, command recording, and submission. Opaque handles should guard against stale references, for example with generation counters.

### Shared ECS components

```cpp
struct Transform {
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};
};

struct Sprite {
    TextureHandle texture;
    glm::vec4 color{1.0f};
};

struct Velocity {
    glm::vec3 linear{0.0f};
};
```

EnTT iteration:

```cpp
auto view = registry.view<Transform, Velocity>();
for (auto entity : view) {
    auto& transform = view.get<Transform>(entity);
    auto& velocity = view.get<Velocity>(entity);
    transform.position += velocity.linear * static_cast<float>(dt);
}
```

2D can use the XY plane with Z-axis rotation; 3D can reuse the transform infrastructure.

## 9. Fixed timestep game loop

```cpp
#include <algorithm>
#include <cmath>

constexpr double fixedDt = 1.0 / 60.0;
constexpr double maxFrameTime = 0.25;
constexpr int maxUpdates = 5;

double accumulator = 0.0;
auto previous = Clock::now();

while (running) {
    auto now = Clock::now();
    double frameTime = std::min(seconds(now - previous), maxFrameTime);
    previous = now;

    pollEvents();
    accumulator += frameTime;

    int updates = 0;
    while (accumulator >= fixedDt && updates < maxUpdates) {
        game.onFixedUpdate(ctx, fixedDt);
        accumulator -= fixedDt;
        ++updates;
    }

    if (updates == maxUpdates && accumulator >= fixedDt) {
        accumulator = std::fmod(accumulator, fixedDt);
    }

    game.onUpdate(ctx, frameTime);
    renderer.beginFrame();
    game.onRender(ctx);
    renderer.endFrame();
}
```

`Clock`, `seconds`, `pollEvents`, and `renderer` are placeholders for engine implementations. A bounded update loop prevents excessive catch-up but intentionally drops simulation time; deterministic networked games may require another policy. Add interpolation for smooth rendering when appropriate.

## 10. SDL_GPU frame sketch

```cpp
SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
SDL_GPUTexture* swapchain = nullptr;

SDL_WaitAndAcquireGPUSwapchainTexture(
    cmd, window, &swapchain, nullptr, nullptr
);

if (swapchain) {
    SDL_GPUColorTargetInfo target{};
    target.texture = swapchain;
    target.load_op = SDL_GPU_LOADOP_CLEAR;
    target.store_op = SDL_GPU_STOREOP_STORE;
    target.clear_color = {0.1f, 0.1f, 0.15f, 1.0f};

    SDL_GPURenderPass* pass =
        SDL_BeginGPURenderPass(cmd, &target, 1, nullptr);

    SDL_BindGPUGraphicsPipeline(pass, pipeline);
    // Bind buffers, samplers, and shader resources here.
    SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
    SDL_EndGPURenderPass(pass);
}

SDL_SubmitGPUCommandBuffer(cmd);
```

This is an **illustrative snippet**, not a standalone executable. It assumes initialized device/window/pipeline and omits error handling and required resource bindings. GPU resource destruction must respect in-flight work and SDL_GPU lifetime rules.

## 11. Shader pipeline

```text
HLSL source (vertex / fragment / compute)
                  |
             SDL_shadercross
                  |
        +---------+---------+
        |         |         |
      SPIR-V     DXIL    Metal-compatible output
        |         |         |
        +---------+---------+
                  |
        Engine shader loader
                  |
             SDL_GPU
```

Compile shaders as part of the CMake asset build, load the appropriate format for the active backend, and defer runtime compilation/hot reload until later. Ensure shader resource declarations and pipeline binding counts agree.

## 12. First project: Breakout

Breakout exercises the game loop, rendering, input, collision, audio, assets, and state management while staying small enough to finish.

### Milestones

1. **Engine bootstrap:** CMake, SDL3, GPU device, swapchain, shaders, first triangle.
2. **Minimal Renderer2D:** orthographic camera, textured and untextured quads, keyboard input, resizing.
3. **Playable Breakout:** paddle, ball, bricks, AABB collision, score, lives, pause.
4. **Reusable systems:** EnTT, texture caching, asset handles, audio, debug UI.
5. **Second game (Asteroids):** validate that the engine isn't Breakout-specific.
6. **3D sandbox:** perspective camera, depth buffer, textured mesh, basic lighting.

### Engine v0.1 acceptance checklist

- [ ] CMake builds a C++23 project.
- [ ] SDL3 opens a 1280 × 720 window.
- [ ] SDL_GPU initializes device and swapchain.
- [ ] Vertex/fragment shader binaries load.
- [ ] A colored triangle appears.
- [ ] Window resize and clean shutdown work.
- [ ] Catch2 tests run via CTest.

### Further games

- **Asteroids:** vectors, rotation, acceleration, entity lifetime, object pooling, particles.
- **Platformer:** tile maps, animation, camera, state machines, level loader.
- **3D exploration:** perspective projection, vertex/index buffers, depth testing, shaders, materials.

## 13. Testing and engineering quality

| Area | Tests and tooling |
|---|---|
| Unit | Vectors, AABB, transforms, clocks |
| Integration | ECS, assets, lifecycle |
| Graphics | Offscreen renders and tolerant image comparisons |
| Resource management | Missing assets, GPU resource lifetime, shutdown |
| Performance | Batching, frame times, entity iteration |
| Safety | AddressSanitizer, UndefinedBehaviorSanitizer |
| Static analysis | clang-tidy, clang-format |
| CI | GitHub Actions on Linux and Windows |

Use deterministic seeds and inputs for simulation tests. RenderDoc is valuable for graphics debugging; platform/window-system details may affect capture support.

## 14. Features to postpone

Avoid early work on networking, scripting, custom allocators, multithreaded rendering, a full editor, render graph, skeletal animation, and physically based rendering. Build two games before committing to large abstractions.

## 15. Book recommendations

1. **Game Engine Architecture — Jason Gregory**, especially the **4th edition, Volume I: Foundations and Core Engine Systems**. Broad engine architecture and infrastructure.
2. **Game Programming Patterns — Robert Nystrom**. Game loop, component patterns, events, object pools, and spatial partitioning. Free online: https://gameprogrammingpatterns.com/
3. **Real-Time Rendering, 4th Edition — Akenine-Möller, Haines, Hoffman**. Advanced 3D rendering reference.
4. **Foundations of Game Engine Development, Volume 1: Mathematics — Eric Lengyel**. Vectors, matrices, transforms, and quaternions.
5. **Game Physics Engine Development — Ian Millington**. Rigid-body physics, collision, and integration.

Suggested order: *Game Programming Patterns* alongside *Game Engine Architecture Volume I*, then Lengyel for 3D math, *Real-Time Rendering* for 3D graphics, and Millington when building physics systems.

The fourth edition of *Game Engine Architecture* is divided into **Volume I: Foundations and Core Engine Systems** and **Volume II: Graphics, Motion, and Sound**. Confirm the edition, volume, ISBN, and current price before purchasing. Publisher: https://www.routledge.com/

Previously discussed Dutch retailer listings (prices and availability are not reverified here):

- Volume I, bol.com: https://www.bol.com/nl/nl/p/game-engine-architecture/9300000235034544/
- Volume II, bol.com: https://www.bol.com/nl/nl/p/game-engine-architecture/9300000235034713/
- Volume I, AwesomeBooks: https://www.awesomebooks.com/book/9781032443089/game-engine-architecture?gc=NL
- Volume I eBook, VitalSource: https://www.vitalsource.com/nl/products/game-engine-architecture-jason-gregory-v9781040731147

## 16. Reference links

- SDL3 documentation: https://wiki.libsdl.org/SDL3/FrontPage
- SDL3 examples: https://examples.libsdl.org/SDL3/
- SDL_GPU API: https://wiki.libsdl.org/SDL3/CategoryGPU
- SDL_GPU examples: https://github.com/TheSpydog/SDL_gpu_examples
- SDL_shadercross: https://github.com/libsdl-org/SDL_shadercross
- LearnOpenGL: https://learnopengl.com/
- Game Programming Patterns: https://gameprogrammingpatterns.com/
- raylib examples: https://www.raylib.com/examples.html

## 17. Final decisions

- Keep the original `Application`, `Game`, `World`, and fixed-timestep concepts.
- Improve `Game` with a `GameContext` rather than globals.
- Separate `Renderer2D` and `Renderer3D` from a lower-level `RenderDevice`.
- Implement only an SDL_GPU backend initially.
- Add shader build tooling and explicit GPU resource lifetime management.
- Keep the original directory layout with dedicated GPU and shader modules.
- Build Breakout first, Asteroids second, and 3D afterward.

**Bottom line:** SDL3 + SDL_GPU + C++23 + CMake is a strong foundation for a custom, reusable 2D/3D game engine. Validate the architecture through finished games instead of trying to build a comprehensive engine upfront.
