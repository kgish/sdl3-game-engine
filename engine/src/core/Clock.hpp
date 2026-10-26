#pragma once

#include <cstdint>

namespace engine {

// Monotonic high-resolution clock backed by SDL's performance counter.
class Clock {
public:
    Clock();

    // Seconds since the previous tick() (or construction).
    double tick();

private:
    std::uint64_t frequency_ = 0;
    std::uint64_t previous_ = 0;
};

} // namespace engine
