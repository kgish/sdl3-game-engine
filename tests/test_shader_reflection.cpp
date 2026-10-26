#include <catch2/catch_test_macros.hpp>

#include "renderer/gpu/GPUShader.hpp"

using engine::gpu::parseShaderResources;

TEST_CASE("Shader reflection JSON from shadercross is parsed", "[shader]")
{
    // Verbatim output of `shadercross color2d.vert.hlsl -o color2d.vert.json`.
    const auto resources = parseShaderResources(
        R"({ "samplers": 0, "storage_textures": 0, "storage_buffers": 0, "uniform_buffers": 1, )"
        R"("inputs": [{ "name": "in.var.TEXCOORD0", "type": "float2", "location": 0 }], "outputs": [] })");

    CHECK(resources.samplers == 0);
    CHECK(resources.storageTextures == 0);
    CHECK(resources.storageBuffers == 0);
    CHECK(resources.uniformBuffers == 1);
}

TEST_CASE("Missing reflection keys default to zero", "[shader]")
{
    const auto resources = parseShaderResources(R"({ "samplers": 2 })");
    CHECK(resources.samplers == 2);
    CHECK(resources.uniformBuffers == 0);
}
