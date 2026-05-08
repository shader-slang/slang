// Emission of atomic intrinsics across HLSL, GLSL, and SPIRV.
// Verifies that InterlockedAdd / InterlockedMin / InterlockedCompareExchange
// lower to the target's native atomic operation set.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly

RWStructuredBuffer<uint> outputBuffer;
groupshared uint sharedCounter;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    uint oldVal;
    InterlockedAdd(outputBuffer[0], 1u, oldVal);
    InterlockedMin(outputBuffer[1], idx, oldVal);
    InterlockedAdd(sharedCounter, idx, oldVal);
    uint cmp;
    InterlockedCompareExchange(outputBuffer[2], 0u, idx, cmp);
}

// CHECK_HLSL-DAG: InterlockedAdd
// CHECK_HLSL-DAG: InterlockedMin
// CHECK_HLSL-DAG: InterlockedCompareExchange

// GLSL spelling for SSBO atomics.
// CHECK_GLSL-DAG: atomicAdd
// CHECK_GLSL-DAG: atomicMin
// CHECK_GLSL-DAG: atomicCompSwap

// SPIRV opcode set for atomics.
// CHECK_SPIRV-DAG: OpAtomicIAdd
// CHECK_SPIRV-DAG: OpAtomicUMin
// CHECK_SPIRV-DAG: OpAtomicCompareExchange
