//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:45+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-program-execution.md#program-execution
//META: doc_section_digest=adbd230383e252b1f6ddca986456ca84146e424a9f5eea3bdd18df8bd52b8168
//META: purpose=The thread group dimensions (group_dim) specified via [numthreads(x,y,z)] on a compute entry point are preserved in emitted HLSL and SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "The thread group size is similarly specified with a 3-dimensional
// vector group_dim. Within the thread group, individual thread invocations b are
// instantiated such that 0 <= b.x < group_dim.x, 0 <= b.y < group_dim.y,
// 0 <= b.z < group_dim.z. The thread group dimensions are typically specified in
// the compute entry point as an attribute."
//
// We verify that [numthreads(8, 4, 2)] emits the correct dimensions in both HLSL
// and SPIR-V (LocalSize 8 4 2).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(8, 4, 2)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    output[0] = tid.x;
}

// HLSL preserves the numthreads dimensions.
// CHECK: [numthreads(8, 4, 2)]

// SPIR-V emits LocalSize with the matching dimensions.
// CHECK-SPV: LocalSize 8 4 2
