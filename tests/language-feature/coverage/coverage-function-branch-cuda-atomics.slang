// Verifies that function and branch coverage markers lower to native
// CUDA atomics on the synthesized coverage buffer.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -stage compute -entry computeMain -trace-function-coverage -trace-branch-coverage

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = tid.x;
    if ((tid.x & 1u) != 0u)
        accum += 1u;
    else
        accum += 2u;
    outputBuffer[0] = accum;
}

// CUDA binds global resources through the generated GlobalParams
// symbol, so function/branch coverage must place the hidden buffer
// there just like line coverage.
//CHECK: struct GlobalParams
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage

// One function-entry marker and two branch-arm markers lower to native
// CUDA atomicAdd operations.
//CHECK-DAG: atomicAdd{{.*_slang_coverage.*int\(0\)}}
//CHECK-DAG: atomicAdd{{.*_slang_coverage.*int\(1\)}}
//CHECK-DAG: atomicAdd{{.*_slang_coverage.*int\(2\)}}
//CHECK-NOT: atomicAdd{{.*_slang_coverage.*int\(3\)}}
