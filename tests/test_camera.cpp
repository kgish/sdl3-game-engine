#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <engine/Renderer.hpp>

using Catch::Approx;

namespace {
glm::vec4 project(const engine::Camera2D& camera, float x, float y)
{
    return camera.viewProjection() * glm::vec4(x, y, 0.0f, 1.0f);
}
} // namespace

TEST_CASE("Camera2D maps its region to clip space with +Y down", "[camera]")
{
    const engine::Camera2D camera{.position = {0.0f, 0.0f}, .size = {1280.0f, 720.0f}};

    const glm::vec4 topLeft = project(camera, 0.0f, 0.0f);
    CHECK(topLeft.x == Approx(-1.0f));
    CHECK(topLeft.y == Approx(1.0f));

    const glm::vec4 bottomRight = project(camera, 1280.0f, 720.0f);
    CHECK(bottomRight.x == Approx(1.0f));
    CHECK(bottomRight.y == Approx(-1.0f));

    const glm::vec4 centre = project(camera, 640.0f, 360.0f);
    CHECK(centre.x == Approx(0.0f).margin(1e-6));
    CHECK(centre.y == Approx(0.0f).margin(1e-6));
}

TEST_CASE("Camera2D depth for z = 0 lies inside SDL_GPU's [0, 1] range", "[camera]")
{
    const engine::Camera2D camera;
    const glm::vec4 p = project(camera, 100.0f, 100.0f);
    CHECK(p.z >= 0.0f);
    CHECK(p.z <= 1.0f);
}

TEST_CASE("Camera2D position scrolls the view", "[camera]")
{
    const engine::Camera2D camera{.position = {100.0f, 50.0f}, .size = {200.0f, 100.0f}};
    const glm::vec4 topLeft = project(camera, 100.0f, 50.0f);
    CHECK(topLeft.x == Approx(-1.0f));
    CHECK(topLeft.y == Approx(1.0f));
}
