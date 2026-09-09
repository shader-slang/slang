//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies `-trace-branch-coverage` makes a `while` loop's condition lower with a coverage marker on each arm, backed by a synthesized `_slang_coverage` counter buffer.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the branch-coverage arms of `visitWhileStmt`. Without the option the
// loop test branches straight to the body and break labels; with it, lowering
// allocates a branch-site id, inserts a fresh block on the false arm so a
// false-arm marker can be emitted before the break, and emits a true-arm
// marker at the top of the body. The observable consequence in HLSL is a
// synthesized `RWStructuredBuffer<uint64_t> _slang_coverage` global plus an
// `InterlockedAdd` on it inside the `else` of the rewritten loop test -- the
// else branch only exists because the false arm got its own block.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute -trace-branch-coverage

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int n = int(tid.x) + 3;
    int total = 0;
    int i = 0;
    while (i < n)
    {
        total += i;
        i++;
    }
    do
    {
        total += 1;
    }
    while (total < 0);
    outputBuffer[0] = total;
}

// The counter buffer is synthesized by the coverage instrumentation.
//CHECK-DAG: RWStructuredBuffer<uint64_t > _slang_coverage_{{[0-9]+}}
// Each loop-test arm bumps a counter in it.
//CHECK-DAG: InterlockedAdd(_slang_coverage_{{[0-9]+}}[{{.*}}], 1ULL
