//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
//META: purpose=Expansion: HLSL meta-module `RWTexture3D.Load(int3 idx)` lowers per target (HLSL Load on RWTexture3D, GLSL imageLoad over image3D, SPIR-V OpImageRead).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `RWTexture3D<T>.Load(int3)` is a sampler-less direct-read on a 3D
// writable image. SPIR-V uses OpImageRead.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWTexture3D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Load(int3(tid));
}

// HLSL: RWTexture3D
// HLSL: Load(
// GLSL: imageLoad(
// SPIRV: OpImageRead
