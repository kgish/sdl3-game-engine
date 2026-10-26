#pragma once

#include <memory>

#include "engine/Game.hpp"

namespace engine {

struct AppConfig {
    const char* title = "SDL3 Game Engine";
    int width = 1280;
    int height = 720;
    bool vsync = true;
    bool gpuDebug = false;  // Enable SDL_GPU validation layers
    int maxFrames = 0;      // Quit after this many frames; 0 runs until closed
};

class Application {
public:
    explicit Application(AppConfig config);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Runs the main loop until the window closes. Returns a process exit code.
    int run(Game& game);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace engine
