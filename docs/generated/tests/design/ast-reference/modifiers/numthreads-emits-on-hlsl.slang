//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#stage-specific-entry-point-attributes
//META: doc_section_digest=9976dfbcc03a512351f63ab9251ef58411b90efdfa2f7539d1c820164c50911b
//META: purpose=`[numthreads(x,y,z)]` workgroup dimensions appear in the emitted entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the attribute's three axis arguments are retained individually and
// reproduced on the emitted entry point. Asymmetric extents (8, 4, 2) are used so
// a mix-up of the axes would change the CHECK text; CHECK asserts the emitted
// HLSL attribute matches the written order.

// WGSL and SPIR-V carry the same three extents in their own entry-point state
// -- `@workgroup_size(8, 4, 2)` and the `LocalSize 8 4 2` execution mode -- so
// each prefix re-checks the per-axis ordering on that back end.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(8, 4, 2)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

//CHECK: numthreads(8, 4, 2)

//WGSL: @workgroup_size(8, 4, 2)
//SPIRV: OpExecutionMode %{{[A-Za-z0-9_]+}} LocalSize 8 4 2
