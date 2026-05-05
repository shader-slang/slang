// Locks in the per-inst-UID design: each `IncrementCoverageCounter` op
// gets its own counter slot, even when multiple ops originate from the
// same source line. The old design deduplicated slots by (file, line),
// which would have collapsed the three statements on line 9 below into
// one or two slots. If someone reintroduces that dedupe, this test
// regresses.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint a = 0; uint b = 1; uint c = 2;
    outputBuffer[0] = a + b + c;
}

// The three statements on one source line must each produce a distinct
// atomic add on a distinct slot. If the compiler dedupes back to
// (file, line), slots 2 and 3 here would disappear.
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(2\)}}
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(3\)}}
