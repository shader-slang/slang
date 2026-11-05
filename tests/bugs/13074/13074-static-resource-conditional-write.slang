//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=D3D): -O0 -target dxil -profile ps_6_0 -entry main -validate-ir
//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV): -target spirv-asm -entry main -stage fragment -validate-ir

//D3D: target cannot represent an opaque value in mutable local storage
//D3D: mutable local storage contains an opaque value of type 'Texture2D', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable
//SPIRV: opaque type in local variable is not supported for Khronos targets
//SPIRV: a resource or other opaque-typed value ('Texture2D') cannot be placed in a function-local variable for Khronos targets (SPIR-V/GLSL) or WGSL

Texture2D<float4> textureA;
Texture2D<float4> textureB;
uniform uint choose;
static Texture2D<float4> selectedTexture = textureA;

[noinline]
void maybeSelectTexture()
{
    if (choose != 0)
        selectedTexture = textureB;
}

[shader("fragment")]
float4 main() : SV_Target
{
    maybeSelectTexture();
    return selectedTexture.Load(int3(0, 0, 0));
}
