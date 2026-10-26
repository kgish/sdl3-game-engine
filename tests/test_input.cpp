#include <catch2/catch_test_macros.hpp>

#include <engine/Input.hpp>

using engine::Input;
using engine::Key;

TEST_CASE("Input tracks pressed, held, and released edges", "[input]")
{
    Input input;

    input.beginFrame();
    input.setKey(Key::Space, true);
    CHECK(input.isDown(Key::Space));
    CHECK(input.wasPressed(Key::Space));

    input.beginFrame();
    CHECK(input.isDown(Key::Space));
    CHECK_FALSE(input.wasPressed(Key::Space));

    input.beginFrame();
    input.setKey(Key::Space, false);
    CHECK_FALSE(input.isDown(Key::Space));
    CHECK(input.wasReleased(Key::Space));

    CHECK_FALSE(input.isDown(Key::Left));
}
