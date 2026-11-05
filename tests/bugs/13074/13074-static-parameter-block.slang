// A parameter block is a binding container rather than an addressable value. Resource-global
// legalization can replace direct resource values and fixed-size resource arrays, but it cannot
// turn a parameter block into one function-local variable. Semantic checking therefore rejects a
// mutable `static` parameter block before lowering tries to allocate storage for it.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry fragmentMain -stage fragment

struct Material
{
    Texture2D<float4> texture;
};

ParameterBlock<Material> material;
static ParameterBlock<Material> cachedMaterial = material;
//CHECK:                        ^^^^^^^^^^^^^^ mutable global variable requires an addressable type
//CHECK:                        ^^^^^^^^^^^^^^ a mutable static global variable requires an addressable type.

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return cachedMaterial.texture.Load(int3(0, 0, 0));
}
