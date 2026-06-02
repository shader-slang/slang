//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:45+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-program-execution.md#thread-group-execution-model
//META: doc_section_digest=b513542addae4e13593e8fe9a226dda934c5e9723c7326c64dc0ad3ead5da188
//META: purpose=All threads within a thread group share local memory allocated with the groupshared attribute; compiler emits the groupshared declaration for a compute entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `groupshared` variables are shared among all threads in a group.
// Thread 0 writes to a groupshared array, all threads barrier and read back; CHECK
// asserts HLSL emits the `groupshared` keyword and SPIR-V emits `Workgroup` storage
// class, confirming the intra-group shared-memory mechanism is correctly lowered.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

groupshared uint shared[4];

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    if (tid.x == 0u)
        shared[0] = 42u;

    GroupMemoryBarrierWithGroupSync();

    outputBuffer[tid.x] = shared[0];
}

// HLSL emits groupshared declaration.
// CHECK: groupshared

// SPIR-V emits Workgroup storage class for the shared variable.
// CHECK-SPV: Workgroup
