//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#hlsl-specific-runtime-predicates
//META: doc_section_digest=05a2ea3acb9ebb0668e90f202fe188cf4ca1f104177b22521b8652d6efbd73eb
//META: purpose=HLSL is non-Khronos: an RWStructuredBuffer is HLSL-shaped (register(uN)) on HLSL but GLSL-shaped (layout buffer) on GLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL is `isD3DTarget` and is filtered into the `default` arms
// of per-target switches that are otherwise Khronos-specific.
// The observable contrast: the same RWStructuredBuffer<int> emits
// `register(u0)` on HLSL and `layout(...) buffer` on GLSL.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: RWStructuredBuffer<int{{.*}}> outBuf_{{[0-9]+}} : register(u{{[0-9]+}})
// GLSL: layout({{.*}}) {{.*}}buffer
