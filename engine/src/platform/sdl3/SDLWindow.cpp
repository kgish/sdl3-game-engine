#include "platform/sdl3/SDLWindow.hpp"

#include <format>
#include <stdexcept>
#include <string>

#include <SDL3/SDL_init.h>

namespace engine::platform {

SDLWindow::SDLWindow(const char* title, int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        throw std::runtime_error(std::format("SDL_Init failed: {}", SDL_GetError()));
    }

    window_ = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (window_ == nullptr) {
        const std::string error = SDL_GetError();
        SDL_Quit();
        throw std::runtime_error(std::format("SDL_CreateWindow failed: {}", error));
    }
}

SDLWindow::~SDLWindow()
{
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

} // namespace engine::platform
