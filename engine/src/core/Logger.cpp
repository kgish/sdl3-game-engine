#include <string>

#include <SDL3/SDL_log.h>

#include "engine/Log.hpp"

namespace engine::log {

void write(Level level, std::string_view message)
{
    SDL_LogPriority priority = SDL_LOG_PRIORITY_INFO;
    switch (level) {
    case Level::Debug: priority = SDL_LOG_PRIORITY_DEBUG; break;
    case Level::Info: priority = SDL_LOG_PRIORITY_INFO; break;
    case Level::Warn: priority = SDL_LOG_PRIORITY_WARN; break;
    case Level::Error: priority = SDL_LOG_PRIORITY_ERROR; break;
    }
    const std::string text(message);
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, priority, "%s", text.c_str());
}

} // namespace engine::log
