#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/FixedTimestep.hpp"

using engine::FixedTimestep;

namespace {
int countSteps(FixedTimestep& timestep)
{
    int steps = 0;
    while (timestep.step()) {
        ++steps;
    }
    return steps;
}
} // namespace

TEST_CASE("Exactly one step per step-sized frame", "[timestep]")
{
    FixedTimestep timestep(0.01);
    for (int frame = 0; frame < 10; ++frame) {
        timestep.advance(0.01);
        CHECK(countSteps(timestep) == 1);
    }
}

TEST_CASE("Short frames accumulate into a step", "[timestep]")
{
    FixedTimestep timestep(0.01);
    timestep.advance(0.004);
    CHECK(countSteps(timestep) == 0);
    timestep.advance(0.004);
    CHECK(countSteps(timestep) == 0);
    timestep.advance(0.004);
    CHECK(countSteps(timestep) == 1);
    CHECK(timestep.alpha() == Catch::Approx(0.2).margin(1e-9));
}

TEST_CASE("Long frames are clamped and catch-up is bounded", "[timestep]")
{
    FixedTimestep timestep(1.0 / 60.0);
    const double used = timestep.advance(5.0);
    CHECK(used == FixedTimestep::kMaxFrameTime);
    CHECK(countSteps(timestep) == FixedTimestep::kMaxUpdates);
    // The backlog beyond the update budget is dropped.
    CHECK(timestep.alpha() < 1.0);
}

TEST_CASE("Negative frame times are ignored", "[timestep]")
{
    FixedTimestep timestep(0.01);
    CHECK(timestep.advance(-1.0) == 0.0);
    CHECK(countSteps(timestep) == 0);
}
