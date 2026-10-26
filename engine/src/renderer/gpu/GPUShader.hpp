#pragma once

#include <cstdint>
#include <filesystem>
#include <string_view>

#include <SDL3/SDL_gpu.h>

namespace engine::gpu {

// Resource counts a shader declares; SDL_GPU needs them at creation time.
struct ShaderResources {
    std::uint32_t samplers = 0;
    std::uint32_t storageTextures = 0;
    std::uint32_t storageBuffers = 0;
    std::uint32_t uniformBuffers = 0;
};

// Parses the reflection JSON written by `shadercross -d JSON`.
ShaderResources parseShaderResources(std::string_view json);

// Loads <dir>/<name>.{spv|dxil|msl} for the device's backend, taking resource
// counts from <dir>/<name>.json. `name` is e.g. "color2d.vert"; the stage
// comes from its .vert/.frag suffix. Throws std::runtime_error on failure.
SDL_GPUShader* loadShader(SDL_GPUDevice* device, const std::filesystem::path& dir, std::string_view name);

} // namespace engine::gpu
