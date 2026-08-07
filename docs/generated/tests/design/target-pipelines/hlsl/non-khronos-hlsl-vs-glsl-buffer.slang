//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#hlsl-specific-runtime-predicates
//META: doc_section_digest=5aff8e92c7894cdefb24e2d630b00a42766e9a4798e9e1fc37d94061ca29ab6e
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
