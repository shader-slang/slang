//TEST:SIMPLE: -target hlsl -entry main -stage fragment -o 13074-static-resource-conditional-write.hlsl
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV): -target spirv-asm -entry main -stage fragment -validate-ir

//SPIRV: opaque type in local variable is not supported for Khronos targets
//SPIRV: a resource or other opaque-typed value ('Texture2D') cannot be placed in a function-local variable for Khronos targets (SPIR-V/GLSL) or WGSL

//CHECK-DAG: Texture2D<float4 > [[TEXTURE_A:textureA_[0-9]+]]
//CHECK-DAG: Texture2D<float4 > [[TEXTURE_B:textureB_[0-9]+]]
//CHECK-LABEL: float4 main()
//CHECK: Texture2D<float4 > [[SELECTED:selectedTexture_[0-9]+]];
//CHECK: if({{.*}}choose
//CHECK: [[SELECTED]] = [[TEXTURE_B]];
//CHECK: else
//CHECK: [[SELECTED]] = [[TEXTURE_A]];
//CHECK: return [[SELECTED]].Load

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
