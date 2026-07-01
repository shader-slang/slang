//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=The HLSL meta-module's `Texture2D` resource type lowers per target — HLSL `Texture2D`, GLSL `texture2D`/sampler binding, SPIR-V `OpTypeImage` / `OpTypeSampledImage`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Texture2D` and `SamplerState` are HLSL meta-module names. Their
// per-target lowering is observable: HLSL keeps the names, GLSL
// emits a sampler binding (`uniform`/`layout`), SPIR-V emits
// `OpTypeImage` plus `OpTypeSampledImage`.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2 uv = float2(float(tid.x), float(tid.y));
    outBuf[tid.x] = tex.SampleLevel(samp, uv, 0.0);
}

// HLSL: Texture2D
// GLSL: {{(texture2D|sampler2D|uniform)}}
// SPIRV: OpTypeImage
