//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:45+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-program-execution.md#program-execution
//META: doc_section_digest=adbd230383e252b1f6ddca986456ca84146e424a9f5eea3bdd18df8bd52b8168
//META: purpose=A compute entry point receives thread coordinates (SV_DispatchThreadID, SV_GroupID, SV_GroupThreadID) identifying each invocation in the 3D dispatch grid.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a compute entry point emits all three thread-coordinate semantics.
// The shader declares SV_DispatchThreadID, SV_GroupID, and SV_GroupThreadID parameters;
// CHECK confirms HLSL emits those semantic names and SPIR-V uses GlobalInvocationId,
// WorkgroupId, and LocalInvocationId, proving the coordinate bindings survive lowering.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> result;

[numthreads(4, 1, 1)]
void main(uint3 dispatchID : SV_DispatchThreadID,
          uint3 groupID    : SV_GroupID,
          uint3 groupThread: SV_GroupThreadID)
{
    result[dispatchID.x] = groupID.x * 4u + groupThread.x;
}

// CHECK: SV_DispatchThreadID
// CHECK: SV_GroupID
// CHECK: SV_GroupThreadID

// CHECK-SPIRV: GlobalInvocationId
// CHECK-SPIRV: WorkgroupId
// CHECK-SPIRV: LocalInvocationId
