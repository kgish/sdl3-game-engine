#include "renderer/gpu/GPUShader.hpp"

#include <charconv>
#include <cstdint>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>

#include <SDL3/SDL.h>

#include "engine/Log.hpp"

namespace engine::gpu {

namespace {

struct SDLFree {
    void operator()(void* p) const { SDL_free(p); }
};

struct FileData {
    std::unique_ptr<void, SDLFree> data;
    std::size_t size = 0;
};

FileData loadFile(const std::filesystem::path& path)
{
    std::size_t size = 0;
    void* data = SDL_LoadFile(path.string().c_str(), &size);
    if (data == nullptr) {
        throw std::runtime_error(std::format("Cannot read '{}': {}", path.string(), SDL_GetError()));
    }
    return {std::unique_ptr<void, SDLFree>(data), size};
}

std::uint32_t readCount(std::string_view json, std::string_view key)
{
    const std::string quoted = std::format("\"{}\"", key);
    std::size_t pos = json.find(quoted);
    if (pos == std::string_view::npos) {
        return 0;
    }
    pos = json.find(':', pos + quoted.size());
    if (pos == std::string_view::npos) {
        return 0;
    }
    pos = json.find_first_not_of(" \t\r\n", pos + 1);
    if (pos == std::string_view::npos) {
        return 0;
    }
    std::uint32_t value = 0;
    std::from_chars(json.data() + pos, json.data() + json.size(), value);
    return value;
}

SDL_GPUShaderStage stageFromName(std::string_view name)
{
    if (name.ends_with(".vert")) {
        return SDL_GPU_SHADERSTAGE_VERTEX;
    }
    if (name.ends_with(".frag")) {
        return SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
    throw std::runtime_error(std::format("Shader name '{}' must end in .vert or .frag", name));
}

} // namespace

ShaderResources parseShaderResources(std::string_view json)
{
    return ShaderResources{
        .samplers = readCount(json, "samplers"),
        .storageTextures = readCount(json, "storage_textures"),
        .storageBuffers = readCount(json, "storage_buffers"),
        .uniformBuffers = readCount(json, "uniform_buffers"),
    };
}

SDL_GPUShader* loadShader(SDL_GPUDevice* device, const std::filesystem::path& dir, std::string_view name)
{
    const SDL_GPUShaderFormat supported = SDL_GetGPUShaderFormats(device);

    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    const char* extension = nullptr;
    const char* entrypoint = "main";
    if ((supported & SDL_GPU_SHADERFORMAT_SPIRV) != 0) {
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        extension = ".spv";
    } else if ((supported & SDL_GPU_SHADERFORMAT_DXIL) != 0) {
        format = SDL_GPU_SHADERFORMAT_DXIL;
        extension = ".dxil";
    } else if ((supported & SDL_GPU_SHADERFORMAT_MSL) != 0) {
        format = SDL_GPU_SHADERFORMAT_MSL;
        extension = ".msl";
        entrypoint = "main0";  // SPIRV-Cross renames main() for Metal
    } else {
        throw std::runtime_error("GPU device supports no shader format the engine builds");
    }

    const std::string base(name);
    const FileData json = loadFile(dir / (base + ".json"));
    const ShaderResources resources =
        parseShaderResources({static_cast<const char*>(json.data.get()), json.size});
    const FileData code = loadFile(dir / (base + extension));

    const SDL_GPUShaderCreateInfo info{
        .code_size = code.size,
        .code = static_cast<const Uint8*>(code.data.get()),
        .entrypoint = entrypoint,
        .format = format,
        .stage = stageFromName(name),
        .num_samplers = resources.samplers,
        .num_storage_textures = resources.storageTextures,
        .num_storage_buffers = resources.storageBuffers,
        .num_uniform_buffers = resources.uniformBuffers,
        .props = 0,
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
    if (shader == nullptr) {
        throw std::runtime_error(std::format("Cannot create shader '{}': {}", name, SDL_GetError()));
    }
    log::debug("Loaded shader {}{}", name, extension);
    return shader;
}

} // namespace engine::gpu
