//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:45+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-program-execution.md#program-execution
//META: doc_section_digest=adbd230383e252b1f6ddca986456ca84146e424a9f5eea3bdd18df8bd52b8168
//META: purpose=SV_GroupID lies in [0,grid_dim) and SV_GroupThreadID lies in [0,group_dim) for each axis of a 3D dispatch.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that SV_GroupID and SV_GroupThreadID stay within their documented 3D bounds.
// A numthreads(2,3,1) kernel dispatched 1x1x1 writes 1 per thread if its GroupID is
// (0,0,0) and its GroupThreadID fits within [0,2)x[0,3)x{0}; CHECK-buffer asserts
// all six output slots are 1, proving no thread escapes the declared extents.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(2, 3, 1)]
void computeMain(uint3 groupID     : SV_GroupID,
                 uint3 groupThread : SV_GroupThreadID)
{
    // Flat index into output: groupThread.y * 2 + groupThread.x
    uint idx = groupThread.y * 2u + groupThread.x;

    // groupID must be (0,0,0) for a 1-group dispatch
    uint groupInRange = (groupID.x == 0u && groupID.y == 0u && groupID.z == 0u) ? 1u : 0u;
    // groupThread.x in [0,2), .y in [0,3), .z == 0
    uint threadInRange = (groupThread.x < 2u && groupThread.y < 3u && groupThread.z == 0u) ? 1u : 0u;

    outputBuffer[idx] = groupInRange & threadInRange;
}

// All 6 threads (2x3) should output 1.
// CHECK: 1
// CHECK: 1
// CHECK: 1
// CHECK: 1
// CHECK: 1
// CHECK: 1
