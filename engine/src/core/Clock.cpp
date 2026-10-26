#include "core/Clock.hpp"

#include <SDL3/SDL_timer.h>

namespace engine {

Clock::Clock() : frequency_(SDL_GetPerformanceFrequency()), previous_(SDL_GetPerformanceCounter()) {}

double Clock::tick()
{
    const std::uint64_t now = SDL_GetPerformanceCounter();
    const double seconds = static_cast<double>(now - previous_) / static_cast<double>(frequency_);
    previous_ = now;
    return seconds;
}

} // namespace engine
