//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
//META: purpose=Expansion: HLSL meta-module `Texture2DMS.Load(int2 coord, sampleIdx)` lowers per target (HLSL Load with sample index, GLSL texelFetch, SPIR-V OpImageFetch over a multisampled image).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Texture2DMS<T>` is multisampled and exposes a `Load(int2, int)`
// taking explicit (xy, sampleIndex) — no mip parameter. SPIR-V records
// MS=1 on the image type.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

Texture2DMS<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Load(int2(int(tid.x), int(tid.y)), 0);
}

// HLSL: Load(
// GLSL: texelFetch(
// SPIRV: OpImageFetch
