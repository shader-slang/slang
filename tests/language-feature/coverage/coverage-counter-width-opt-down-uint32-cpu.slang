// Verifies that `-trace-coverage-counter-width 32` on the CPU
// target routes the coverage atomic-add through the uint32 prelude
// helper (`_slang_atomic_add_u32`) rather than the uint64 helper
// (`_slang_atomic_add_u64`).
//
// The CPU target's default is uint64 (covered by the sibling
// `coverage-cpu-dispatch.slang`); this test is the regression guard
// for the opt-down path that customers on width-constrained runtimes
// rely on.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage -trace-coverage-counter-width 32

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = tid.x;
}

// Coverage buffer present.
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage

// uint32 helper called; the uint64 helper must NOT appear.
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage
//CHECK-NOT: _slang_atomic_add_u64
