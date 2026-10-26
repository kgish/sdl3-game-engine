// Breakout, milestone 1 (engine bootstrap): a colored triangle plus a paddle
// driven by the fixed-timestep update, proving window, GPU, shaders, input,
// and the game loop work end to end. Gameplay arrives in milestone 3.

#include <algorithm>
#include <exception>

#include <engine/Application.hpp>
#include <engine/Input.hpp>
#include <engine/Log.hpp>
#include <engine/Renderer.hpp>

namespace {

constexpr float kWorldWidth = 1280.0f;
constexpr float kWorldHeight = 720.0f;
constexpr float kPaddleSpeed = 900.0f;  // world units per second
constexpr glm::vec2 kPaddleSize{160.0f, 20.0f};

class Breakout final : public engine::Game {
public:
    void onInit(engine::GameContext& ctx) override
    {
        ctx.renderer2D.setClearColor({0.08f, 0.08f, 0.12f, 1.0f});
        engine::log::info("Breakout bootstrap: arrows/A/D move the paddle, Esc quits");
    }

    void onFixedUpdate(engine::GameContext& ctx, double dt) override
    {
        float direction = 0.0f;
        if (ctx.input.isDown(engine::Key::Left) || ctx.input.isDown(engine::Key::A)) {
            direction -= 1.0f;
        }
        if (ctx.input.isDown(engine::Key::Right) || ctx.input.isDown(engine::Key::D)) {
            direction += 1.0f;
        }
        const float halfWidth = kPaddleSize.x / 2.0f;
        paddleX_ = std::clamp(paddleX_ + direction * kPaddleSpeed * static_cast<float>(dt), halfWidth,
                              kWorldWidth - halfWidth);
    }

    void onUpdate(engine::GameContext& ctx, double /*dt*/) override
    {
        if (ctx.input.wasPressed(engine::Key::Escape)) {
            running_ = false;
        }
    }

    void onRender(engine::GameContext& ctx) override
    {
        engine::Renderer2D& r = ctx.renderer2D;
        r.beginScene(camera_);

        r.drawTriangle({{
            {.position = {kWorldWidth / 2.0f, 120.0f}, .color = {1.0f, 0.2f, 0.2f, 1.0f}},
            {.position = {kWorldWidth / 2.0f + 260.0f, 520.0f}, .color = {0.2f, 1.0f, 0.2f, 1.0f}},
            {.position = {kWorldWidth / 2.0f - 260.0f, 520.0f}, .color = {0.2f, 0.4f, 1.0f, 1.0f}},
        }});

        r.drawRect({paddleX_, kWorldHeight - 40.0f}, kPaddleSize, {0.9f, 0.9f, 0.95f, 1.0f});

        r.endScene();
    }

    [[nodiscard]] bool isRunning() const override { return running_; }

private:
    engine::Camera2D camera_{.position = {0.0f, 0.0f}, .size = {kWorldWidth, kWorldHeight}};
    float paddleX_ = kWorldWidth / 2.0f;
    bool running_ = true;
};

} // namespace

int main(int /*argc*/, char* /*argv*/[])
{
    try {
        engine::Application app({.title = "Breakout", .width = 1280, .height = 720, .vsync = true});
        Breakout game;
        return app.run(game);
    } catch (const std::exception& e) {
        engine::log::error("Startup failed: {}", e.what());
        return 1;
    }
}
