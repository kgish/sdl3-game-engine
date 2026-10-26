#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include <glm/vec4.hpp>

#include "engine/Types.hpp"

namespace engine {

enum class BufferUsage { Vertex, Index };

struct BufferDesc {
    BufferUsage usage = BufferUsage::Vertex;
    std::uint32_t size = 0;  // bytes
};

enum class VertexFormat { Float2, Float3, Float4 };

struct VertexAttribute {
    std::uint32_t location = 0;  // matches TEXCOORD<n> in the HLSL vertex input
    VertexFormat format = VertexFormat::Float4;
    std::uint32_t offset = 0;
};

enum class PrimitiveType { TriangleList, LineList };

struct PipelineDesc {
    // Shader base names in the shader directory, e.g. "color2d.vert".
    std::string vertexShader;
    std::string fragmentShader;
    std::vector<VertexAttribute> attributes;
    std::uint32_t vertexStride = 0;
    PrimitiveType primitive = PrimitiveType::TriangleList;
    bool alphaBlend = true;
};

// Backend-neutral GPU interface used by the renderers. Gameplay code never
// sees it. Only an SDL_GPU implementation exists; the interface keeps the
// renderers testable and SDL_GPU types out of their headers.
//
// Per frame:  beginFrame() -> [uploadBuffer()...] -> beginRenderPass() ->
//             bind/push/draw... -> endRenderPass() -> ... -> endFrame()
// Uploads must happen outside render passes.
class RenderDevice {
public:
    virtual ~RenderDevice() = default;

    virtual BufferHandle createBuffer(const BufferDesc& desc) = 0;
    virtual PipelineHandle createPipeline(const PipelineDesc& desc) = 0;
    virtual void destroyBuffer(BufferHandle handle) = 0;
    virtual void destroyPipeline(PipelineHandle handle) = 0;

    // Returns false when there is nothing to draw into (e.g. minimised
    // window); the caller should skip rendering but still call endFrame().
    virtual bool beginFrame() = 0;
    virtual void endFrame() = 0;
    [[nodiscard]] virtual bool frameActive() const = 0;

    virtual void setClearColor(const glm::vec4& color) = 0;
    virtual void uploadBuffer(BufferHandle buffer, std::span<const std::byte> data, std::uint32_t offset = 0) = 0;

    // The first pass of a frame clears the swapchain; later passes load it.
    virtual void beginRenderPass() = 0;
    virtual void endRenderPass() = 0;
    virtual void bindPipeline(PipelineHandle pipeline) = 0;
    virtual void bindVertexBuffer(BufferHandle buffer, std::uint32_t slot = 0) = 0;
    virtual void pushVertexUniforms(std::uint32_t slot, std::span<const std::byte> data) = 0;
    virtual void draw(std::uint32_t vertexCount, std::uint32_t firstVertex = 0) = 0;
};

} // namespace engine
