// Verifies that function and branch coverage markers lower to native
// SPIR-V atomics on the synthesized coverage buffer.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-function-coverage -trace-branch-coverage

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

// The synthesized coverage buffer must be present for function/branch
// coverage even when line coverage is disabled.
//CHECK: __slang_coverage

// One function-entry marker and two branch-arm markers lower through
// the same storage-buffer atomic path used by line coverage.
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_0
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_1
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_2
//CHECK-DAG: OpAtomicIAdd
//CHECK-DAG: OpAtomicIAdd
//CHECK-DAG: OpAtomicIAdd
//CHECK-NOT: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_3
