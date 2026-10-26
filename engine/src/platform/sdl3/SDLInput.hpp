#pragma once

#include "engine/Input.hpp"

namespace engine::platform {

// Drains the SDL event queue into `input`. Returns false once the user has
// asked to quit (window closed, Ctrl+C, ...).
bool pollEvents(Input& input);

} // namespace engine::platform
