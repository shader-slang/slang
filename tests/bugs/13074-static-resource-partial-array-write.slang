//TEST:SIMPLE: -target hlsl -entry main -stage fragment -o 13074-static-resource-partial-array-write.hlsl
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV): -target spirv-asm -entry main -stage fragment -validate-ir

//SPIRV: opaque type in local variable is not supported for Khronos targets
//SPIRV: a resource or other opaque-typed value ('Texture2D') cannot be placed in a function-local variable for Khronos targets (SPIR-V/GLSL) or WGSL

//CHECK: Texture2D<float4 >  [[TEXTURES:textures_[0-9]+]][int(3)]
//CHECK-LABEL: float4 main()
//CHECK: Texture2D<float4 >  [[CACHED:cachedTextures_[0-9]+]][int(2)]
//CHECK: [[CACHED]][int(0)] = [[TEXTURES]][int(0)];
//CHECK: [[CACHED]][int(1)] = [[TEXTURES]][int(1)];
//CHECK: [[CACHED]][int(1)] = [[TEXTURES]][int(2)];
//CHECK: return [[CACHED]][int(0)].Load({{.*}}) + [[CACHED]][int(1)].Load

Texture2D<float4> textures[3];
static Texture2D<float4> cachedTextures[2] = {textures[0], textures[1]};

[noinline]
void updateCachedTexture()
{
    cachedTextures[1] = textures[2];
}

[shader("fragment")]
float4 main() : SV_Target
{
    updateCachedTexture();
    return cachedTextures[0].Load(int3(0, 0, 0)) +
           cachedTextures[1].Load(int3(0, 0, 0));
}
