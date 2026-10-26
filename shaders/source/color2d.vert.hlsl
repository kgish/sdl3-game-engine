// Untextured 2D geometry: per-vertex color, transformed by the camera.
//
// SDL_GPU binding convention for vertex shaders:
//   uniform buffers -> register(b[n], space1)

cbuffer Camera : register(b0, space1)
{
    float4x4 viewProjection;
};

struct Input
{
    float2 position : TEXCOORD0;
    float4 color    : TEXCOORD1;
};

struct Output
{
    float4 color    : TEXCOORD0;
    float4 position : SV_Position;
};

Output main(Input input)
{
    Output output;
    output.color = input.color;
    output.position = mul(viewProjection, float4(input.position, 0.0, 1.0));
    return output;
}
