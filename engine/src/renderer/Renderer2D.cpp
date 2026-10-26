#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>

#include <glm/ext/matrix_transform.hpp>

#include "engine/Renderer.hpp"
#include "renderer/RenderDevice.hpp"

namespace engine {

namespace {

constexpr std::size_t kInitialVertexCapacity = 6 * 1024;

std::span<const std::byte> bytesOf(const auto& value)
{
    return std::as_bytes(std::span{&value, 1});
}

} // namespace

Renderer2D::Renderer2D(RenderDevice& device) : device_(device)
{
    pipeline_ = device_.createPipeline(PipelineDesc{
        .vertexShader = "color2d.vert",
        .fragmentShader = "color2d.frag",
        .attributes =
            {
                {.location = 0, .format = VertexFormat::Float2, .offset = offsetof(Vertex2D, position)},
                {.location = 1, .format = VertexFormat::Float4, .offset = offsetof(Vertex2D, color)},
            },
        .vertexStride = sizeof(Vertex2D),
        .primitive = PrimitiveType::TriangleList,
        .alphaBlend = true,
    });
    vertices_.reserve(kInitialVertexCapacity);
}

Renderer2D::~Renderer2D()
{
    device_.destroyBuffer(vertexBuffer_);
    device_.destroyPipeline(pipeline_);
}

void Renderer2D::setClearColor(const glm::vec4& color)
{
    device_.setClearColor(color);
}

void Renderer2D::beginScene(const Camera2D& camera)
{
    if (inScene_) {
        throw std::logic_error("Renderer2D::beginScene called twice without endScene");
    }
    inScene_ = true;
    viewProjection_ = camera.viewProjection();
    vertices_.clear();
}

void Renderer2D::drawTriangle(const std::array<Vertex2D, 3>& vertices)
{
    vertices_.insert(vertices_.end(), vertices.begin(), vertices.end());
}

void Renderer2D::drawRect(const glm::mat4& transform, const glm::vec4& color)
{
    const auto corner = [&](float x, float y) {
        const glm::vec4 p = transform * glm::vec4(x, y, 0.0f, 1.0f);
        return Vertex2D{.position = {p.x, p.y}, .color = color};
    };
    const Vertex2D topLeft = corner(-0.5f, -0.5f);
    const Vertex2D topRight = corner(0.5f, -0.5f);
    const Vertex2D bottomRight = corner(0.5f, 0.5f);
    const Vertex2D bottomLeft = corner(-0.5f, 0.5f);
    drawTriangle({topLeft, topRight, bottomRight});
    drawTriangle({topLeft, bottomRight, bottomLeft});
}

void Renderer2D::drawRect(glm::vec2 center, glm::vec2 size, const glm::vec4& color)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    transform = glm::scale(transform, glm::vec3(size, 1.0f));
    drawRect(transform, color);
}

void Renderer2D::endScene()
{
    if (!inScene_) {
        throw std::logic_error("Renderer2D::endScene called without beginScene");
    }
    inScene_ = false;

    if (vertices_.empty() || !device_.frameActive()) {
        return;
    }

    // Grow the GPU buffer geometrically; the old one is released once the
    // GPU is done with it.
    if (vertices_.size() > vertexCapacity_) {
        device_.destroyBuffer(vertexBuffer_);
        vertexCapacity_ = std::max(vertices_.size(), std::max(kInitialVertexCapacity, vertexCapacity_ * 2));
        vertexBuffer_ = device_.createBuffer(BufferDesc{
            .usage = BufferUsage::Vertex,
            .size = static_cast<std::uint32_t>(vertexCapacity_ * sizeof(Vertex2D)),
        });
    }

    device_.uploadBuffer(vertexBuffer_, std::as_bytes(std::span{vertices_}));
    device_.beginRenderPass();
    device_.bindPipeline(pipeline_);
    device_.bindVertexBuffer(vertexBuffer_);
    device_.pushVertexUniforms(0, bytesOf(viewProjection_));
    device_.draw(static_cast<std::uint32_t>(vertices_.size()));
    device_.endRenderPass();
}

} // namespace engine
