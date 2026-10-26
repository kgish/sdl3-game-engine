#include "engine/Application.hpp"

#include <charconv>
#include <cstring>
#include <exception>
#include <filesystem>

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>

#include "core/Clock.hpp"
#include "core/FixedTimestep.hpp"
#include "engine/Assets.hpp"
#include "engine/Input.hpp"
#include "engine/Log.hpp"
#include "engine/Renderer.hpp"
#include "engine/World.hpp"
#include "platform/sdl3/SDLInput.hpp"
#include "platform/sdl3/SDLWindow.hpp"
#include "renderer/gpu/SDLGPURenderDevice.hpp"

namespace engine {

namespace {

std::filesystem::path basePath()
{
    const char* base = SDL_GetBasePath();  // owned by SDL
    return base != nullptr ? std::filesystem::path(base) : std::filesystem::current_path();
}

// ENGINE_MAX_FRAMES=<n> overrides AppConfig::maxFrames; used by smoke tests.
int maxFramesOverride(int fallback)
{
    const char* value = SDL_getenv("ENGINE_MAX_FRAMES");
    if (value == nullptr) {
        return fallback;
    }
    int frames = fallback;
    std::from_chars(value, value + std::strlen(value), frames);
    return frames;
}

} // namespace

// Members are declared in dependency order so they are destroyed in reverse:
// renderers before the GPU device, the device before the window.
struct Application::Impl {
    explicit Impl(const AppConfig& config)
        : window(config.title, config.width, config.height),
          device(window.handle(),
                 gpu::DeviceConfig{
                     .vsync = config.vsync,
                     .debug = config.gpuDebug,
                     .shaderDir = basePath() / "shaders",
                 }),
          renderer2D(device),
          assets(basePath()),
          maxFrames(maxFramesOverride(config.maxFrames))
    {
    }

    platform::SDLWindow window;
    gpu::SDLGPURenderDevice device;
    Renderer2D renderer2D;
    World world;
    Input input;
    Assets assets;
    int maxFrames = 0;
};

Application::Application(AppConfig config) : impl_(std::make_unique<Impl>(config)) {}

Application::~Application() = default;

int Application::run(Game& game)
{
    Impl& app = *impl_;
    GameContext ctx{
        .renderer2D = app.renderer2D,
        .world = app.world,
        .input = app.input,
        .assets = app.assets,
    };

    try {
        game.onInit(ctx);

        Clock clock;
        FixedTimestep timestep;
        int frame = 0;

        while (platform::pollEvents(app.input) && game.isRunning()) {
            const double frameTime = timestep.advance(clock.tick());

            while (timestep.step()) {
                game.onFixedUpdate(ctx, timestep.stepSize());
            }
            game.onUpdate(ctx, frameTime);

            if (app.device.beginFrame()) {
                game.onRender(ctx);
            }
            app.device.endFrame();

            if (app.maxFrames > 0 && ++frame >= app.maxFrames) {
                break;
            }
        }

        game.onShutdown(ctx);
    } catch (const std::exception& e) {
        log::error("Fatal: {}", e.what());
        return 1;
    }
    return 0;
}

} // namespace engine
