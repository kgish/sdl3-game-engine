#pragma once

#include <SDL3/SDL_video.h>

namespace engine::platform {

// Owns SDL initialisation and the main window.
class SDLWindow {
public:
    // Throws std::runtime_error if SDL or the window cannot be created.
    SDLWindow(const char* title, int width, int height);
    ~SDLWindow();

    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;

    [[nodiscard]] SDL_Window* handle() const { return window_; }

private:
    SDL_Window* window_ = nullptr;
};

} // namespace engine::platform
