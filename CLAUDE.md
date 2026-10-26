# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

C++23 game engine on SDL3 (platform/input) and SDL_GPU (rendering via Vulkan/D3D12/Metal). `SDL3_GAME_ENGINE.md` is the design document: architecture principles, technology stack, and the milestone roadmap (Breakout → Asteroids → 3D sandbox). Read it before making architectural changes. The current state is milestone 1 (engine bootstrap).

## Commands

```sh
cmake --preset debug                 # configure (presets: debug, release, asan, tidy, full)
cmake --build --preset debug         # builds engine, shaders, games, tests
ctest --preset debug                 # all tests, incl. GPU smoke test (needs display + GPU)
ctest --preset headless              # skips tests labelled "gpu"
ctest --preset debug -R "Stale handles"          # one test by name (Catch2 cases are discovered as CTest tests)
./build/debug/bin/engine_tests "[timestep]"      # Catch2 directly, by tag or test name
ENGINE_MAX_FRAMES=60 ./build/debug/bin/breakout  # run N frames then exit
```

- `asan` preset: ASan+UBSan. Its tests use `tools/sanitizers/lsan.supp` to hide leaks in system libraries (dbus, libdecor, GPU drivers). Pass `LSAN_OPTIONS=suppressions=$PWD/tools/sanitizers/lsan.supp` when running binaries by hand.
- `full` preset turns on every optional `ENGINE_WITH_*` library. Use it to check dependency upgrades; `tests/test_thirdparty.cpp` has one smoke test per library.
- `tidy` preset needs `clang-tidy` installed. Formatting follows `.clang-format`.

## Build system

- **Shader toolchain bootstrap:** `cmake/Shadercross.cmake` looks for the `shadercross` CLI. If it isn't found, the first configure builds `tools/shadercross/` (a pinned ExternalProject superbuild of SDL3, SPIRV-Cross, SDL_shadercross and prebuilt DXC) and installs it into `external/shadercross/`. All presets share that install. Delete the directory to force a rebuild, or override it with `-DSHADERCROSS_EXECUTABLE=`. The bootstrap only supports Linux x86_64 and Windows x64.
- **Shaders:** HLSL in `shaders/source/` must be named `<name>.<vert|frag|comp>.hlsl` and listed in `shaders/CMakeLists.txt`. `engine_add_shaders()` (`cmake/Shaders.cmake`) writes `<name>.<stage>.json` (reflection) and `.spv` into `build/<preset>/bin/shaders/`. It adds `.dxil` on Windows, `.msl` on Apple, or every format with `ENGINE_SHADERS_ALL_FORMATS`. Follow the SDL_GPU register conventions, e.g. vertex-stage uniform buffers are `register(b[n], space1)`. Vertex inputs use `TEXCOORD<n>`, which maps to attribute location n.
- **Dependencies:** `cmake/Dependencies.cmake` fetches everything with FetchContent from URL archives pinned with SHA256. SDL3 is linked statically. When bumping a version, update the URL and the hash. Optional libraries link through the `engine::thirdparty` interface target and define `ENGINE_HAS_<NAME>`.
- Call `engine_target_defaults(<target>)` (`cmake/CompilerOptions.cmake`) on every first-party target. It applies warnings, sanitizers and clang-tidy. Never apply these to third-party targets.

## Architecture

The design rules that span several files:

- **Games depend only on `engine::engine`** and the public headers in `engine/include/engine/`. Gameplay code never includes SDL or calls SDL_GPU. SDL3 is a PRIVATE dependency of the engine. GLM is public because it appears in the public headers.
- **Rendering layers:** `Renderer2D` (public, `Renderer.hpp`) → `RenderDevice` (private abstract interface, `engine/src/renderer/RenderDevice.hpp`) → `gpu::SDLGPURenderDevice`, the only backend. Renderers talk to the GPU only through `RenderDevice`.
- **Frame protocol** (enforced by `SDLGPURenderDevice`, which throws `std::logic_error` on misuse): `beginFrame()` → uploads → `beginRenderPass()` → bind/push/draw → `endRenderPass()` → `endFrame()`. Uploads (copy passes) are not allowed inside a render pass. The first render pass of a frame clears the swapchain and later passes load it. If nothing drew, `endFrame()` clears anyway. `beginFrame()` returns false when there is no swapchain texture (e.g. a minimised window), and `Application` then skips `onRender`.
- **Shader loading** (`renderer/gpu/GPUShader.cpp`): it picks SPIR-V, then DXIL, then MSL, whichever the device supports. MSL uses entry point `main0`. Resource counts for `SDL_CreateGPUShader` come from the `.json` reflection file, so never hardcode binding counts. Pipelines refer to shaders by base name, e.g. `"color2d.vert"`. The shader directory is `SDL_GetBasePath()/shaders`.
- **Resources** are identified by generation-checked `Handle<Tag>` (`Types.hpp`) and stored in `HandlePool` (`core/HandlePool.hpp`). A default handle is null. A stale handle resolves to nullptr.
- **`Renderer2D`** collects vertices between `beginScene`/`endScene` and submits them as one draw. It grows its vertex buffer geometrically and pushes the camera matrix as vertex uniform slot 0. `Camera2D` is orthographic with +Y down, and its world region `[position, position+size]` is stretched over the window. GLM is compiled with `GLM_FORCE_DEPTH_ZERO_TO_ONE` to match SDL_GPU clip space. The C++ code passes GLM column-major matrices as-is, and the HLSL side uses `mul(matrix, vector)`.
- **Lifetime/ownership:** `Application::Impl` declares its members in dependency order (window → device → renderer → services), so they are destroyed in reverse. `GameContext` holds references to these services. The device calls `SDL_WaitForGPUIdle` before releasing resources. Individual releases rely on SDL deferring destruction until in-flight command buffers finish.
- **Loop:** `core/FixedTimestep.hpp` runs 60 Hz `onFixedUpdate`, clamps frame time to 0.25 s, and allows at most 5 catch-up steps, dropping any further backlog. `onUpdate`/`onRender` run once per frame. A game ends the loop by returning false from `Game::isRunning()`.
- Tests can include private engine headers: `engine/src` is on the test include path.

## Conventions from the design doc

- Build only the abstractions the current milestone needs. Each optional library stays off until its milestone. Postpone networking, scripting, custom allocators, multithreaded rendering, a render graph, and PBR.
- Keep physics optional and separate. 2D and 3D share the asset, input, ECS and GPU-resource infrastructure.
