//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:45+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-program-execution.md#program-execution
//META: doc_section_digest=adbd230383e252b1f6ddca986456ca84146e424a9f5eea3bdd18df8bd52b8168
//META: purpose=Total invocations equal grid_dim.x*group_dim.x (for 1D case); each thread receives a unique SV_DispatchThreadID with 0 <= tid.x < group_dim.x.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states total invocations per dispatch = grid_dim.x * grid_dim.y * grid_dim.z
// * group_dim.x * group_dim.y * group_dim.z.
// With numthreads(4,1,1) and a 1x1x1 grid, there are 4 total threads.
// Each thread writes its flat dispatch thread ID (x component) to its output slot.
// We verify thread IDs 0, 1, 2, 3 are assigned: tid.x is in [0, group_dim.x).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = tid.x;
}

// Thread IDs 0 through 3 are assigned, verifying the range [0, group_dim.x).
// CHECK: 0
// CHECK: 1
// CHECK: 2
// CHECK: 3
