//TEST:SIMPLE: -target spirv -entry main -stage fragment -o 13074-resource-dependent-initializer.spv
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment

//CHECK-LABEL: ; Function main
//CHECK: %[[TEXTURE:[A-Za-z0-9_]+]] = OpLoad {{.*}} %texture
//CHECK: %[[VALUE:[A-Za-z0-9_]+]] = OpImageFetch %v4float %[[TEXTURE]]
//CHECK: OpStore %entryPointParam_main %[[VALUE]]

Texture2D<float4> texture;
static Texture2D<float4> cachedTexture = texture;
static float4 cachedValue = cachedTexture.Load(int3(0, 0, 0));

[shader("fragment")]
float4 main() : SV_Target
{
    return cachedValue;
}
