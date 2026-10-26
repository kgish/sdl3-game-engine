#include "renderer/gpu/SDLGPURenderDevice.hpp"

#include <algorithm>
#include <cstring>
#include <format>
#include <stdexcept>

#include "engine/Log.hpp"
#include "renderer/gpu/GPUShader.hpp"

namespace engine::gpu {

namespace {

SDL_GPUVertexElementFormat toSDL(VertexFormat format)
{
    switch (format) {
    case VertexFormat::Float2: return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    case VertexFormat::Float3: return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    case VertexFormat::Float4: return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
    }
    return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
}

SDL_GPUPrimitiveType toSDL(PrimitiveType type)
{
    switch (type) {
    case PrimitiveType::TriangleList: return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    case PrimitiveType::LineList: return SDL_GPU_PRIMITIVETYPE_LINELIST;
    }
    return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
}

SDL_GPUBufferUsageFlags toSDL(BufferUsage usage)
{
    switch (usage) {
    case BufferUsage::Vertex: return SDL_GPU_BUFFERUSAGE_VERTEX;
    case BufferUsage::Index: return SDL_GPU_BUFFERUSAGE_INDEX;
    }
    return SDL_GPU_BUFFERUSAGE_VERTEX;
}

[[noreturn]] void fail(std::string_view what)
{
    throw std::runtime_error(std::format("{}: {}", what, SDL_GetError()));
}

} // namespace

SDLGPURenderDevice::SDLGPURenderDevice(SDL_Window* window, DeviceConfig config)
    : window_(window), config_(std::move(config))
{
    // Request every format the build can produce; SDL picks the best backend
    // (Vulkan, Direct3D 12, or Metal) the platform supports.
    const SDL_GPUShaderFormat formats =
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
    device_ = SDL_CreateGPUDevice(formats, config_.debug, nullptr);
    if (device_ == nullptr) {
        fail("SDL_CreateGPUDevice failed");
    }

    if (!SDL_ClaimWindowForGPUDevice(device_, window_)) {
        SDL_DestroyGPUDevice(device_);
        fail("SDL_ClaimWindowForGPUDevice failed");
    }

    // VSYNC is always supported. Without vsync prefer MAILBOX (no tearing),
    // then IMMEDIATE.
    SDL_GPUPresentMode presentMode = SDL_GPU_PRESENTMODE_VSYNC;
    if (!config_.vsync) {
        if (SDL_WindowSupportsGPUPresentMode(device_, window_, SDL_GPU_PRESENTMODE_MAILBOX)) {
            presentMode = SDL_GPU_PRESENTMODE_MAILBOX;
        } else if (SDL_WindowSupportsGPUPresentMode(device_, window_, SDL_GPU_PRESENTMODE_IMMEDIATE)) {
            presentMode = SDL_GPU_PRESENTMODE_IMMEDIATE;
        }
    }
    SDL_SetGPUSwapchainParameters(device_, window_, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, presentMode);

    log::info("SDL_GPU device created (driver: {})", SDL_GetGPUDeviceDriver(device_));
}

SDLGPURenderDevice::~SDLGPURenderDevice()
{
    // Make sure no submitted work still references the resources.
    SDL_WaitForGPUIdle(device_);

    buffers_.forEach([this](Buffer& buffer) { SDL_ReleaseGPUBuffer(device_, buffer.gpu); });
    pipelines_.forEach([this](SDL_GPUGraphicsPipeline* pipeline) { SDL_ReleaseGPUGraphicsPipeline(device_, pipeline); });
    buffers_.clear();
    pipelines_.clear();
    if (transferBuffer_ != nullptr) {
        SDL_ReleaseGPUTransferBuffer(device_, transferBuffer_);
    }

    SDL_ReleaseWindowFromGPUDevice(device_, window_);
    SDL_DestroyGPUDevice(device_);
}

BufferHandle SDLGPURenderDevice::createBuffer(const BufferDesc& desc)
{
    const SDL_GPUBufferCreateInfo info{.usage = toSDL(desc.usage), .size = desc.size, .props = 0};
    SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(device_, &info);
    if (buffer == nullptr) {
        fail("SDL_CreateGPUBuffer failed");
    }
    return buffers_.insert(Buffer{buffer, desc.size});
}

PipelineHandle SDLGPURenderDevice::createPipeline(const PipelineDesc& desc)
{
    SDL_GPUShader* vertexShader = loadShader(device_, config_.shaderDir, desc.vertexShader);
    SDL_GPUShader* fragmentShader = nullptr;
    try {
        fragmentShader = loadShader(device_, config_.shaderDir, desc.fragmentShader);
    } catch (...) {
        SDL_ReleaseGPUShader(device_, vertexShader);
        throw;
    }

    const SDL_GPUVertexBufferDescription vertexBuffer{
        .slot = 0,
        .pitch = desc.vertexStride,
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0,
    };

    std::vector<SDL_GPUVertexAttribute> attributes;
    attributes.reserve(desc.attributes.size());
    for (const VertexAttribute& attribute : desc.attributes) {
        attributes.push_back(SDL_GPUVertexAttribute{
            .location = attribute.location,
            .buffer_slot = 0,
            .format = toSDL(attribute.format),
            .offset = attribute.offset,
        });
    }

    SDL_GPUColorTargetDescription colorTarget{};
    colorTarget.format = SDL_GetGPUSwapchainTextureFormat(device_, window_);
    if (desc.alphaBlend) {
        SDL_GPUColorTargetBlendState& blend = colorTarget.blend_state;
        blend.enable_blend = true;
        blend.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        blend.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        blend.color_blend_op = SDL_GPU_BLENDOP_ADD;
        blend.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        blend.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        blend.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    }

    SDL_GPUGraphicsPipelineCreateInfo info{};
    info.vertex_shader = vertexShader;
    info.fragment_shader = fragmentShader;
    info.vertex_input_state.vertex_buffer_descriptions = &vertexBuffer;
    info.vertex_input_state.num_vertex_buffers = 1;
    info.vertex_input_state.vertex_attributes = attributes.data();
    info.vertex_input_state.num_vertex_attributes = static_cast<Uint32>(attributes.size());
    info.primitive_type = toSDL(desc.primitive);
    info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    info.target_info.color_target_descriptions = &colorTarget;
    info.target_info.num_color_targets = 1;

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device_, &info);

    // Pipelines keep what they need; the shader objects can go.
    SDL_ReleaseGPUShader(device_, vertexShader);
    SDL_ReleaseGPUShader(device_, fragmentShader);

    if (pipeline == nullptr) {
        fail("SDL_CreateGPUGraphicsPipeline failed");
    }
    return pipelines_.insert(pipeline);
}

void SDLGPURenderDevice::destroyBuffer(BufferHandle handle)
{
    // SDL defers the actual release until in-flight command buffers finish.
    if (auto buffer = buffers_.remove(handle)) {
        SDL_ReleaseGPUBuffer(device_, buffer->gpu);
    }
}

void SDLGPURenderDevice::destroyPipeline(PipelineHandle handle)
{
    if (auto pipeline = pipelines_.remove(handle)) {
        SDL_ReleaseGPUGraphicsPipeline(device_, *pipeline);
    }
}

bool SDLGPURenderDevice::beginFrame()
{
    commandBuffer_ = SDL_AcquireGPUCommandBuffer(device_);
    if (commandBuffer_ == nullptr) {
        fail("SDL_AcquireGPUCommandBuffer failed");
    }
    swapchainCleared_ = false;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer_, window_, &swapchainTexture_, nullptr, nullptr)) {
        fail("SDL_WaitAndAcquireGPUSwapchainTexture failed");
    }
    // A null texture (e.g. minimised window) is not an error: skip drawing.
    return swapchainTexture_ != nullptr;
}

void SDLGPURenderDevice::endFrame()
{
    if (commandBuffer_ == nullptr) {
        return;
    }
    if (renderPass_ != nullptr) {
        endRenderPass();
    }
    // Nothing drew this frame: still clear so the window isn't left with
    // stale or undefined contents.
    if (swapchainTexture_ != nullptr && !swapchainCleared_) {
        beginRenderPass();
        endRenderPass();
    }
    if (!SDL_SubmitGPUCommandBuffer(commandBuffer_)) {
        log::error("SDL_SubmitGPUCommandBuffer failed: {}", SDL_GetError());
    }
    commandBuffer_ = nullptr;
    swapchainTexture_ = nullptr;
}

void SDLGPURenderDevice::ensureTransferCapacity(std::uint32_t size)
{
    if (transferBuffer_ != nullptr && transferCapacity_ >= size) {
        return;
    }
    if (transferBuffer_ != nullptr) {
        SDL_ReleaseGPUTransferBuffer(device_, transferBuffer_);
    }
    transferCapacity_ = std::max(size, transferCapacity_ * 2);
    const SDL_GPUTransferBufferCreateInfo info{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = transferCapacity_,
        .props = 0,
    };
    transferBuffer_ = SDL_CreateGPUTransferBuffer(device_, &info);
    if (transferBuffer_ == nullptr) {
        fail("SDL_CreateGPUTransferBuffer failed");
    }
}

void SDLGPURenderDevice::uploadBuffer(BufferHandle handle, std::span<const std::byte> data, std::uint32_t offset)
{
    if (commandBuffer_ == nullptr || renderPass_ != nullptr) {
        throw std::logic_error("uploadBuffer must be called inside a frame and outside a render pass");
    }
    const Buffer* buffer = buffers_.get(handle);
    if (buffer == nullptr) {
        throw std::logic_error("uploadBuffer: stale or invalid buffer handle");
    }
    const auto size = static_cast<std::uint32_t>(data.size());
    if (size == 0) {
        return;
    }
    if (offset + size > buffer->size) {
        throw std::out_of_range("uploadBuffer: data exceeds buffer size");
    }

    ensureTransferCapacity(size);

    // cycle = true: if the GPU is still reading the previous contents, SDL
    // hands us fresh memory instead of stalling or overwriting it.
    void* mapped = SDL_MapGPUTransferBuffer(device_, transferBuffer_, true);
    if (mapped == nullptr) {
        fail("SDL_MapGPUTransferBuffer failed");
    }
    std::memcpy(mapped, data.data(), size);
    SDL_UnmapGPUTransferBuffer(device_, transferBuffer_);

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer_);
    const SDL_GPUTransferBufferLocation source{.transfer_buffer = transferBuffer_, .offset = 0};
    const SDL_GPUBufferRegion destination{.buffer = buffer->gpu, .offset = offset, .size = size};
    SDL_UploadToGPUBuffer(copyPass, &source, &destination, true);
    SDL_EndGPUCopyPass(copyPass);
}

void SDLGPURenderDevice::beginRenderPass()
{
    if (commandBuffer_ == nullptr || swapchainTexture_ == nullptr || renderPass_ != nullptr) {
        throw std::logic_error("beginRenderPass: no frame, no swapchain texture, or pass already open");
    }
    SDL_GPUColorTargetInfo target{};
    target.texture = swapchainTexture_;
    target.load_op = swapchainCleared_ ? SDL_GPU_LOADOP_LOAD : SDL_GPU_LOADOP_CLEAR;
    target.store_op = SDL_GPU_STOREOP_STORE;
    target.clear_color = SDL_FColor{clearColor_.r, clearColor_.g, clearColor_.b, clearColor_.a};

    renderPass_ = SDL_BeginGPURenderPass(commandBuffer_, &target, 1, nullptr);
    if (renderPass_ == nullptr) {
        fail("SDL_BeginGPURenderPass failed");
    }
    swapchainCleared_ = true;
}

void SDLGPURenderDevice::endRenderPass()
{
    if (renderPass_ != nullptr) {
        SDL_EndGPURenderPass(renderPass_);
        renderPass_ = nullptr;
    }
}

void SDLGPURenderDevice::bindPipeline(PipelineHandle handle)
{
    SDL_GPUGraphicsPipeline** pipeline = pipelines_.get(handle);
    if (renderPass_ == nullptr || pipeline == nullptr) {
        throw std::logic_error("bindPipeline: no render pass or invalid pipeline handle");
    }
    SDL_BindGPUGraphicsPipeline(renderPass_, *pipeline);
}

void SDLGPURenderDevice::bindVertexBuffer(BufferHandle handle, std::uint32_t slot)
{
    const Buffer* buffer = buffers_.get(handle);
    if (renderPass_ == nullptr || buffer == nullptr) {
        throw std::logic_error("bindVertexBuffer: no render pass or invalid buffer handle");
    }
    const SDL_GPUBufferBinding binding{.buffer = buffer->gpu, .offset = 0};
    SDL_BindGPUVertexBuffers(renderPass_, slot, &binding, 1);
}

void SDLGPURenderDevice::pushVertexUniforms(std::uint32_t slot, std::span<const std::byte> data)
{
    if (commandBuffer_ == nullptr) {
        throw std::logic_error("pushVertexUniforms called outside a frame");
    }
    SDL_PushGPUVertexUniformData(commandBuffer_, slot, data.data(), static_cast<Uint32>(data.size()));
}

void SDLGPURenderDevice::draw(std::uint32_t vertexCount, std::uint32_t firstVertex)
{
    if (renderPass_ == nullptr) {
        throw std::logic_error("draw called outside a render pass");
    }
    SDL_DrawGPUPrimitives(renderPass_, vertexCount, 1, firstVertex, 0);
}

} // namespace engine::gpu
