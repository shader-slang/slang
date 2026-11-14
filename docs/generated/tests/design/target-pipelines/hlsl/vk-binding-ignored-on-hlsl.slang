//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#hlsl-specific-runtime-predicates
//META: doc_section_digest=5aff8e92c7894cdefb24e2d630b00a42766e9a4798e9e1fc37d94061ca29ab6e
//META: purpose=Boundary: [vk::binding(N, M)] attributes do not pin HLSL registers — the resource still emits at the positional register(u0).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: vk::binding(7, 2) edge — HLSL is non-Khronos
// and is in the default arm of the per-target switch, so the
// Vulkan binding attribute does not propagate into the HLSL
// register decoration. The binding stays positional.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

[vk::binding(7, 2)]
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: RWStructuredBuffer<int{{.*}}> outBuf_{{[0-9]+}} : register(u0)
