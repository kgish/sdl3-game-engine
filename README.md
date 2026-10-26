# sdl3-game-engine

A reusable C++23 game engine built on **SDL3** (platform, window, input) and
**SDL_GPU** (rendering on Vulkan, Direct3D 12, and Metal). It starts with 2D and
adds 3D later. Design notes, library choices, and the roadmap are in
[SDL3_GAME_ENGINE.md](SDL3_GAME_ENGINE.md).

Status: **engine v0.1, the bootstrap milestone.** The Breakout target opens a
window, draws a colored triangle and a movable paddle through the batched
`Renderer2D`, and shuts down cleanly.

## Requirements

| Tool | Version | Notes |
|---|---|---|
| CMake | ≥ 3.28 | |
| Ninja | any recent | the presets use it |
| C++ compiler | GCC 14+, Clang 18+, MSVC 17.10+ | C++23 with `<format>` |
| Vulkan driver | | Linux (e.g. `mesa-vulkan-drivers` or the NVIDIA driver) |
| Network | first configure only | sources are fetched at pinned versions |

On Linux, SDL3 also needs the windowing and audio development headers:

```sh
sudo apt install build-essential cmake ninja-build \
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxss-dev libxtst-dev \
  libwayland-dev libxkbcommon-dev wayland-protocols libdecor-0-dev \
  libasound2-dev libpulse-dev libpipewire-0.3-dev libudev-dev libdbus-1-dev
```

Optional: `clang-format`, `clang-tidy`, RenderDoc for GPU debugging, and
`vulkan-validationlayers` for `AppConfig::gpuDebug`.

You don't need to install anything else. SDL3, GLM, and Catch2 are fetched by
CMake. The **SDL_shadercross** CLI has no binary releases, so the first
configure builds it from pinned sources (SDL3 + SPIRV-Cross + prebuilt DXC)
into `external/shadercross/`. That takes about a minute and later configures
reuse it. To use your own build instead, pass `-DSHADERCROSS_EXECUTABLE=<path>`.

## Build and run

```sh
cmake --preset debug           # configure (first run also builds shadercross)
cmake --build --preset debug   # engine, shaders, Breakout, tests
ctest --preset debug           # unit tests + a 120-frame GPU smoke test
./build/debug/bin/breakout     # arrows / A / D move the paddle, Esc quits
```

| Preset | Purpose |
|---|---|
| `debug` / `release` | Normal development builds |
| `asan` | AddressSanitizer + UndefinedBehaviorSanitizer (not on Windows) |
| `tidy` | Clang build that runs clang-tidy on engine code |
| `full` | Debug with every optional library on, to check dependency updates |
| `headless` (test only) | Skips tests labelled `gpu` (no display/GPU needed) |

## Layout

```text
CMakeLists.txt, CMakePresets.json
cmake/
  CompilerOptions.cmake   warnings, sanitizers, clang-tidy (first-party targets only)
  Dependencies.cmake      FetchContent: day-one deps + opt-in ENGINE_WITH_* stack
  Shadercross.cmake       finds or bootstraps the shadercross CLI
  Shaders.cmake           engine_add_shaders(): HLSL -> SPIR-V / DXIL / MSL + JSON
engine/
  include/engine/         public API: Application, Game, Renderer, Input, World, Assets, Types, Log
  src/core/               main loop, Clock, FixedTimestep, HandlePool, logging
  src/platform/sdl3/      SDL window and event handling
  src/renderer/           Renderer2D, Camera2D, RenderDevice interface
  src/renderer/gpu/       SDL_GPU implementation of RenderDevice + shader loader
games/breakout/           first game (milestones 1–3)
shaders/source/           HLSL (<name>.<vert|frag|comp>.hlsl)
tests/                    Catch2 unit tests, run through CTest
tools/shadercross/        superbuild for the shadercross CLI
external/                 generated: bootstrapped tools (git-ignored)
```

Compiled shaders go to `build/<preset>/bin/shaders/`, next to the executables.
At runtime the engine picks the format the active backend needs, and it reads
resource counts from the `.json` reflection file that shadercross writes. The
binding counts therefore come from the shader and can't drift from it.

## Architecture rules

- Games link `engine::engine` only and never call SDL or SDL_GPU directly.
- `Renderer2D` (and later `Renderer3D`) sit on the backend-neutral
  `RenderDevice`. `SDLGPURenderDevice` is the only implementation.
- GPU resources are referenced by generation-checked handles, so a stale
  handle fails to resolve instead of aliasing a new resource.
- The fixed-timestep loop uses 60 Hz updates and clamps frame time and
  catch-up (`FixedTimestep`).

## Optional libraries

Each one is off until a milestone needs it. Enable it at configure time, e.g.
`cmake --preset debug -DENGINE_WITH_ENTT=ON`. Enabled libraries link through
`engine::thirdparty` and define `ENGINE_HAS_<NAME>`.

| Option | Library | Milestone |
|---|---|---|
| `ENGINE_WITH_ENTT` | EnTT 4.0.0 | Reusable systems |
| `ENGINE_WITH_BOX2D` | Box2D 3.2.0 | When AABB collision isn't enough |
| `ENGINE_WITH_JOLT` | Jolt Physics 5.6.0 | 3D |
| `ENGINE_WITH_MINIAUDIO` | miniaudio 0.11.25 | First sound effects (or use SDL3 audio) |
| `ENGINE_WITH_IMGUI` | Dear ImGui 1.93.0 with SDL3 + SDL_GPU backends | Debug UI |
| `ENGINE_WITH_STB` | stb_image | Textures |
| `ENGINE_WITH_TINYGLTF` | tinygltf 2.9.7 | 3D models |
| `ENGINE_WITH_JSON` | nlohmann/json 3.12.0 | Scenes and config |
| `ENGINE_WITH_SPDLOG` | spdlog 1.17.0 | When `engine::log` isn't enough |
| `ENGINE_WITH_TRACY` | Tracy 0.14.1 client | Performance work |

Day-one versions: SDL 3.4.18, GLM 1.0.3, Catch2 3.16.1, SDL_shadercross
`1ff05be`, DXC 1.9.2609, SPIRV-Cross vulkan-sdk-1.4.363.0.

## Roadmap

1. ~~Engine bootstrap: CMake, SDL3, GPU device, swapchain, shaders, first triangle~~
2. Minimal Renderer2D: textured quads, texture uploads, resize/letterboxing
3. Playable Breakout
4. Reusable systems: EnTT, asset cache, audio, ImGui
5. Asteroids
6. 3D sandbox: Renderer3D, depth buffer, meshes, lighting
