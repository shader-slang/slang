//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#eliminatephis-with-default-options
//META: doc_section_digest=552c08e936c00c01fc40525a781af719987034dc141a0640ccef762362e861d5
//META: purpose=CUDA uses eliminatePhis with default options so an if/else-merged value becomes a named local temporary assigned in each branch.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// CUDA's `eliminatePhis` runs with the default `PhiEliminationOptions`.
// An if/else merge is materialised as a function-local
// variable that is assigned in each branch; nvrtc re-SSA's it.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(2, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x;
    if (tid.x == 0U)
        x = 100;
    else
        x = 200;
    outBuf[tid.x] = x;
}

// CHECK: int {{[A-Za-z_][A-Za-z0-9_]*}};
// CHECK: if(
// CHECK: = int(100)
// CHECK: else
// CHECK: = int(200)
