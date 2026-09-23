// Resource-type legalization can map one parameter block to separate non-resource data and resource
// values, with details that depend on the target. Because resource-global legalization cannot yet
// replace one file- or namespace-scope `static` variable with several values, semantic checking
// rejects this declaration before legalization.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry fragmentMain -stage fragment

struct Material
{
    Texture2D<float4> texture;
};

ParameterBlock<Material> material;
static ParameterBlock<Material> cachedMaterial = material;
//CHECK:                        ^^^^^^^^^^^^^^ global variable cannot have opaque type
//CHECK:                        ^^^^^^^^^^^^^^ global variable cannot have opaque type.

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return cachedMaterial.texture.Load(int3(0, 0, 0));
}
