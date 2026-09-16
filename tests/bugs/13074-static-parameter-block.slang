//TEST:SIMPLE: -target spirv -entry fragmentMain -stage fragment -validate-ir -o 13074-static-parameter-block.spv

struct Material
{
    Texture2D<float4> texture;
};

ParameterBlock<Material> material;
static ParameterBlock<Material> cachedMaterial = material;

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return cachedMaterial.texture.Load(int3(0, 0, 0));
}
