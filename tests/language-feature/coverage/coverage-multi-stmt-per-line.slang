// Locks in how multiple statements on one source line are handled.
//
// Each `IncrementCoverageCounter` op keeps its own source coverage
// entry, with its own column, so the metadata never deduplicates by
// (file, line) — the three statements on line 24 below produce three
// entries. That entry-level distinctness is asserted by the
// `coverageTracingMetadata` unit test, which reads the metadata API.
//
// What they do *not* each get is a counter. All four statements here
// sit in one basic block, so they provably execute the same number of
// times and are coalesced onto a single counter and a single probe.
// This is what keeps instrumented shader code small; per-line reporting
// is unaffected, because hosts accumulate per entry and every entry
// still reads this one slot.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint a = 0; uint b = 1; uint c = 2;
    outputBuffer[0] = a + b + c;
}

// One straight-line region means exactly one atomic add, on slot 0.
//CHECK: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}

// If coalescing regresses to one counter per statement, slot 1 appears
// and this fails.
//CHECK-NOT: _slang_coverage{{.*int\(1\)}}
