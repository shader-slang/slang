// Verifies that `-trace-coverage-counter-width 32` opts the
// synthesized coverage buffer down to uint32. This is the path for
// runtime drivers that do not support 64-bit shader atomic add
// (most notably MoltenVK on Apple Silicon as of MoltenVK 1.4, which
// reports `shaderBufferInt64Atomics = false`).
//
// The complementary default-uint64 path is in
// `coverage-counter-width-default-uint64-spirv.slang`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-coverage-counter-width 32

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = tid.x;
}

// The opt-down path must NOT declare `Int64Atomics`. If the IR pass
// regresses and continues to emit a 64-bit atomic op despite the
// flag, the SPIR-V emitter's capability auto-declaration would put
// this back in the module.
//CHECK-NOT: OpCapability Int64Atomics

//CHECK: __slang_coverage

// uint32 element layout: 32-bit pointer type and ArrayStride 4.
//CHECK-DAG: OpTypeInt 32 0
//CHECK-DAG: ArrayStride 4

// `OpAtomicIAdd` must operate on the 32-bit `uint` type, not `ulong`.
//CHECK: OpAtomicIAdd %uint
