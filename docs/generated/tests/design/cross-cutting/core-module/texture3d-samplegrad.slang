//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
//META: purpose=Expansion: HLSL meta-module `Texture3D.SampleGrad(sampler, uvw, ddx, ddy)` lowers per target (HLSL SampleGrad, GLSL textureGrad over sampler3D, SPIR-V OpImageSampleExplicitLod with Grad operand).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `SampleGrad` takes user-supplied derivatives, decoupling the call
// site from implicit-derivative requirements. The 3D variant takes
// float3 derivatives.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

Texture3D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 uvw = float3(0.5, 0.5, 0.5);
    float3 ddx = float3(0.01, 0.0, 0.0);
    float3 ddy = float3(0.0, 0.01, 0.0);
    outBuf[tid.x] = tex.SampleGrad(samp, uvw, ddx, ddy);
}

// HLSL: SampleGrad(
// GLSL: textureGrad(sampler3D
// SPIRV: OpImageSampleExplicitLod {{.*}} Grad
