//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -validate-ir

// Check that sampler and texture array selections use nonuniform indices in SPIR-V and GLSL.

// CHECK: OpCapability SampledImageArrayNonUniformIndexing
// CHECK: OpDecorate %t NonUniform
// CHECK: %t = OpSelect %uint
// CHECK: OpAccessChain {{.*}} %textures %t
// CHECK: OpImageSampleImplicitLod
// GLSL: texture(sampler2D(textures_0[nonuniformEXT

cbuffer Ubo { uint i; uint j; };
SamplerState samplers[8];
Texture2D<float4> textures[8];

float4 main(float2 uv : TEXCOORD) : SV_TARGET
{
    // Varying control flow needs a nonuniform index even though each arm can use a constant.
    SamplerState s = uv.x < 0.5 ? samplers[0] : samplers[1];
    Texture2D<float4> t = select(uv.y < 0.5, textures[i], textures[j]);
    return t.Sample(s, uv);
}
