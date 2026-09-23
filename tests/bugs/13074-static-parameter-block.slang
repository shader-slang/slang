// Resource-type legalization represents one parameter block with separate ordinary-data and
// resource values. Resource-global legalization cannot yet replace those values as one file-scope
// `static` variable, so it rejects this declaration before legalization.
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
//CHECK:                        ^^^^^^^^^^^^^^ do you intend to define a `static const` instead?

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return cachedMaterial.texture.Load(int3(0, 0, 0));
}
