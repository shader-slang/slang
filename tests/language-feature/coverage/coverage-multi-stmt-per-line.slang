// Locks in how multiple statements on one source line are handled.
//
// Repeated statements on one source line describe one line execution.
// They produce one canonical line entry, while different lines that
// execute together may share its runtime slot. Consumers must use each
// entry's counterIndex and deduplicate aliases per source line.

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
