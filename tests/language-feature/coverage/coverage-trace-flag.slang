// Verifies the `-trace-coverage` flag on a shader that does NOT
// declare `__slang_coverage` itself — the compiler must synthesize
// the buffer automatically and rewrite the AST-lowering placeholder
// IncrementCoverageCounter ops into counter writes.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = 0;
    for (uint i = 0; i < 4; ++i)
    {
        if ((i & 1u) == 0u)
            accum += i;
        else
            accum += i * 2u;
    }
    outputBuffer[0] = accum;
}

// Synthesized coverage buffer should appear in the emitted C++.
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage

// Each basic block's counter increment materializes as a pointer
// fetch followed by a non-atomic load-add-store on that slot. At
// least three distinct counter indices should show up.
//CHECK: _slang_coverage{{.*int\(0\)}}
//CHECK: _slang_coverage{{.*int\(1\)}}
//CHECK: _slang_coverage{{.*int\(2\)}}
