#include "platform/sdl3/SDLInput.hpp"

#include <optional>

#include <SDL3/SDL_events.h>

namespace engine::platform {

namespace {

std::optional<Key> toKey(SDL_Scancode scancode)
{
    switch (scancode) {
    case SDL_SCANCODE_LEFT: return Key::Left;
    case SDL_SCANCODE_RIGHT: return Key::Right;
    case SDL_SCANCODE_UP: return Key::Up;
    case SDL_SCANCODE_DOWN: return Key::Down;
    case SDL_SCANCODE_W: return Key::W;
    case SDL_SCANCODE_A: return Key::A;
    case SDL_SCANCODE_S: return Key::S;
    case SDL_SCANCODE_D: return Key::D;
    case SDL_SCANCODE_SPACE: return Key::Space;
    case SDL_SCANCODE_RETURN: return Key::Enter;
    case SDL_SCANCODE_ESCAPE: return Key::Escape;
    case SDL_SCANCODE_P: return Key::P;
    default: return std::nullopt;
    }
}

} // namespace

bool pollEvents(Input& input)
{
    input.beginFrame();

    bool running = true;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            if (const auto key = toKey(event.key.scancode)) {
                input.setKey(*key, event.key.down);
            }
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            // Avoid keys that stay "held" because their release went elsewhere.
            for (std::size_t i = 0; i < static_cast<std::size_t>(Key::Count); ++i) {
                input.setKey(static_cast<Key>(i), false);
            }
            break;
        default:
            break;
        }
    }
    return running;
}

} // namespace engine::platform
