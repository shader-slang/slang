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

// At least three distinct counter indices should be written to via
// the CPU-prelude atomic helper. This assertion matches both the
// slot reference AND the real atomic emission so the test would fail
// if codegen regressed to a non-atomic load-add-store.
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(2\)}}
