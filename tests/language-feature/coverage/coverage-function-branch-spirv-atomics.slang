// Verifies that function and branch coverage markers lower to native
// SPIR-V atomics on the synthesized coverage buffer, including the
// structured-CFG shapes used for loop and switch branch instrumentation.

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
    for (uint i = 0; i < 2; ++i)
    {
        accum += i;
    }
    switch (int(tid.x) & 1)
    {
    case 0:
        accum += 10u;
        break;
    default:
        accum += 20u;
        break;
    }
    switch (int(tid.x) & 3)
    {
    case 2:
        accum += 30u;
        break;
    }
    outputBuffer[0] = accum;
}

// The synthesized coverage buffer must be present for function/branch
// coverage even when line coverage is disabled.
//CHECK: __slang_coverage

// One function-entry marker and eight branch-arm markers lower through
// the same storage-buffer atomic path used by line coverage.
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_0
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_1
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_2
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_3
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_4
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_5
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_6
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_7
//CHECK-DAG: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_8
//CHECK-DAG: OpLoopMerge
//CHECK-DAG: OpSelectionMerge
//CHECK-DAG: OpSelectionMerge
//CHECK-DAG: OpAtomicIAdd
//CHECK-DAG: OpAtomicIAdd
//CHECK-DAG: OpAtomicIAdd
//CHECK-DAG: OpAtomicIAdd
//CHECK-DAG: OpAtomicIAdd
//CHECK-DAG: OpAtomicIAdd
//CHECK-DAG: OpAtomicIAdd
//CHECK-DAG: OpAtomicIAdd
//CHECK-DAG: OpAtomicIAdd
//CHECK-NOT: OpAccessChain {{.*}} %__slang_coverage {{.*}} %int_9
