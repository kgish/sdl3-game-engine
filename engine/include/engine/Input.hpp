#pragma once

#include <bitset>
#include <cstddef>

namespace engine {

// Keys the engine exposes to games. Mapped from SDL scancodes, so they refer
// to physical key positions independent of keyboard layout.
enum class Key : std::size_t {
    Left, Right, Up, Down,
    W, A, S, D,
    Space, Enter, Escape, P,
    Count
};

class Input {
public:
    // Held this frame.
    [[nodiscard]] bool isDown(Key key) const { return current_[index(key)]; }
    // Went down this frame.
    [[nodiscard]] bool wasPressed(Key key) const { return current_[index(key)] && !previous_[index(key)]; }
    // Went up this frame.
    [[nodiscard]] bool wasReleased(Key key) const { return !current_[index(key)] && previous_[index(key)]; }

    // Called by the platform layer.
    void beginFrame() { previous_ = current_; }
    void setKey(Key key, bool down) { current_[index(key)] = down; }

private:
    static constexpr std::size_t index(Key key) { return static_cast<std::size_t>(key); }

    std::bitset<static_cast<std::size_t>(Key::Count)> current_;
    std::bitset<static_cast<std::size_t>(Key::Count)> previous_;
};

} // namespace engine
