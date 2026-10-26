#pragma once

#include <filesystem>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "core/HandlePool.hpp"
#include "renderer/RenderDevice.hpp"

namespace engine::gpu {

struct DeviceConfig {
    bool vsync = true;
    bool debug = false;
    std::filesystem::path shaderDir;
};

class SDLGPURenderDevice final : public RenderDevice {
public:
    // Creates the GPU device and claims `window` for presentation.
    // Throws std::runtime_error on failure.
    SDLGPURenderDevice(SDL_Window* window, DeviceConfig config);
    ~SDLGPURenderDevice() override;

    SDLGPURenderDevice(const SDLGPURenderDevice&) = delete;
    SDLGPURenderDevice& operator=(const SDLGPURenderDevice&) = delete;

    BufferHandle createBuffer(const BufferDesc& desc) override;
    PipelineHandle createPipeline(const PipelineDesc& desc) override;
    void destroyBuffer(BufferHandle handle) override;
    void destroyPipeline(PipelineHandle handle) override;

    bool beginFrame() override;
    void endFrame() override;
    [[nodiscard]] bool frameActive() const override { return commandBuffer_ != nullptr; }

    void setClearColor(const glm::vec4& color) override { clearColor_ = color; }
    void uploadBuffer(BufferHandle buffer, std::span<const std::byte> data, std::uint32_t offset) override;

    void beginRenderPass() override;
    void endRenderPass() override;
    void bindPipeline(PipelineHandle pipeline) override;
    void bindVertexBuffer(BufferHandle buffer, std::uint32_t slot) override;
    void pushVertexUniforms(std::uint32_t slot, std::span<const std::byte> data) override;
    void draw(std::uint32_t vertexCount, std::uint32_t firstVertex) override;

private:
    struct Buffer {
        SDL_GPUBuffer* gpu = nullptr;
        std::uint32_t size = 0;
    };

    void ensureTransferCapacity(std::uint32_t size);

    SDL_Window* window_ = nullptr;
    SDL_GPUDevice* device_ = nullptr;
    DeviceConfig config_;

    HandlePool<BufferTag, Buffer> buffers_;
    HandlePool<PipelineTag, SDL_GPUGraphicsPipeline*> pipelines_;

    SDL_GPUTransferBuffer* transferBuffer_ = nullptr;
    std::uint32_t transferCapacity_ = 0;

    // Per-frame state
    SDL_GPUCommandBuffer* commandBuffer_ = nullptr;
    SDL_GPUTexture* swapchainTexture_ = nullptr;
    SDL_GPURenderPass* renderPass_ = nullptr;
    bool swapchainCleared_ = false;
    glm::vec4 clearColor_{0.1f, 0.1f, 0.15f, 1.0f};
};

} // namespace engine::gpu
