#pragma once

namespace engine {

class Renderer2D;
class World;
class Input;
class Assets;

// Engine services handed to the game. All references are owned by the
// Application and stay valid from onInit() until onShutdown() returns.
struct GameContext {
    Renderer2D& renderer2D;
    World& world;
    Input& input;
    Assets& assets;
};

class Game {
public:
    virtual ~Game() = default;
    virtual void onInit(GameContext& /*ctx*/) {}
    virtual void onFixedUpdate(GameContext& /*ctx*/, double /*dt*/) {}
    virtual void onUpdate(GameContext& /*ctx*/, double /*dt*/) {}
    virtual void onRender(GameContext& /*ctx*/) {}
    virtual void onShutdown(GameContext& /*ctx*/) {}

    // Return false to leave the main loop at the end of the current frame.
    [[nodiscard]] virtual bool isRunning() const { return true; }
};

} // namespace engine
