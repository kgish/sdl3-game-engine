#pragma once

#include <algorithm>
#include <cmath>

namespace engine {

// Fixed-timestep accumulator (see "Fix Your Timestep!").
//
// Frame time is clamped and catch-up is bounded, so a long stall drops
// simulation time instead of spiralling. Deterministic networked games may
// need a different policy.
class FixedTimestep {
public:
    static constexpr double kDefaultStep = 1.0 / 60.0;
    static constexpr double kMaxFrameTime = 0.25;
    static constexpr int kMaxUpdates = 5;

    explicit FixedTimestep(double step = kDefaultStep) : step_(step) {}

    // Adds a frame's elapsed time; returns the (clamped) frame time used.
    double advance(double frameTime)
    {
        frameTime = std::clamp(frameTime, 0.0, kMaxFrameTime);
        accumulator_ += frameTime;
        updates_ = 0;
        return frameTime;
    }

    // True while another fixed update should run this frame.
    bool step()
    {
        if (accumulator_ >= step_ && updates_ < kMaxUpdates) {
            accumulator_ -= step_;
            ++updates_;
            if (updates_ == kMaxUpdates && accumulator_ >= step_) {
                accumulator_ = std::fmod(accumulator_, step_);  // drop the backlog
            }
            return true;
        }
        return false;
    }

    [[nodiscard]] double stepSize() const { return step_; }
    // Fraction of a step left over, for interpolating rendered state.
    [[nodiscard]] double alpha() const { return accumulator_ / step_; }

private:
    double step_;
    double accumulator_ = 0.0;
    int updates_ = 0;
};

} // namespace engine
