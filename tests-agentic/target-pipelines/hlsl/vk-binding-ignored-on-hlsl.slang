//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#hlsl-specific-runtime-predicates
//META: doc_section_digest=05a2ea3acb9ebb0668e90f202fe188cf4ca1f104177b22521b8652d6efbd73eb
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
