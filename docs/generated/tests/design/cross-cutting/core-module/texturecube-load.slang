//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
//META: purpose=Expansion: HLSL meta-module `TextureCube.Load(int4 face-major coord + mip)` is accepted by the front-end and lowered (HLSL keeps Load on TextureCube).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Load` on a cube takes int4: three coordinate components + mip
// level. The expectation here is that Slang accepts the call (it is a
// documented HLSL-meta-module overload) and the HLSL backend re-emits
// it verbatim.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

TextureCube<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Load(int4(0, 0, 0, 0));
}

// HLSL: TextureCube
// HLSL: Load(
