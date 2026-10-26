#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "engine/Types.hpp"

namespace engine {

class RenderDevice;

// Orthographic 2D camera in world units, with +Y pointing down.
// The visible region is [position, position + size]; it is stretched to fill
// the window (letterboxing comes later).
struct Camera2D {
    glm::vec2 position{0.0f};
    glm::vec2 size{1280.0f, 720.0f};

    [[nodiscard]] glm::mat4 viewProjection() const;
};

struct Vertex2D {
    glm::vec2 position;
    glm::vec4 color;
};

// Batched 2D renderer. All draws between beginScene() and endScene() are
// collected on the CPU and submitted as a single draw call.
class Renderer2D {
public:
    explicit Renderer2D(RenderDevice& device);
    ~Renderer2D();

    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;

    void setClearColor(const glm::vec4& color);

    void beginScene(const Camera2D& camera);

    void drawTriangle(const std::array<Vertex2D, 3>& vertices);

    // Unit square centred on the origin, placed by `transform`.
    void drawRect(const glm::mat4& transform, const glm::vec4& color);
    // Axis-aligned rectangle given by its centre and size in world units.
    void drawRect(glm::vec2 center, glm::vec2 size, const glm::vec4& color);

    void endScene();

private:
    RenderDevice& device_;
    PipelineHandle pipeline_;
    BufferHandle vertexBuffer_;
    std::size_t vertexCapacity_ = 0;
    std::vector<Vertex2D> vertices_;
    glm::mat4 viewProjection_{1.0f};
    bool inScene_ = false;
};

} // namespace engine
