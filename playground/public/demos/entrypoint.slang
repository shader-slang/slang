// This example demonstrates how to define vertex and fragment shader entrypoints.
// This is a compile-only example, so there is no output from the playground.
// You can use the "Compile" button to compile this example to different targets.

struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
}

struct Material
{
    int albedoTextureIndex;
    int normalTextureIndex;
}

struct Scene
{
    StructuredBuffer<Material> materials;
    Texture2D textures[128];
}

struct Camera
{
    float4x4 mvp;
}

ParameterBlock<Scene> scene;
ConstantBuffer<Camera> camera;
SamplerState samplerState;

struct VOut
{
    float4 position : SV_Position;
    float3 normal;
    float2 uv;
}

[shader("vertex")]
VOut vertexMain(Vertex input)
{
    VOut output;
    output.position = mul(camera.mvp, float4(input.position, 1.0));
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VOut input) : SV_Target
{
    let texture = scene.textures[scene.materials[0].albedoTextureIndex];
    let color = texture.Sample(samplerState, input.uv);
    return color;
}
