// Verifies that the default coverage counter width on SPIR-V targets
// is uint64. The synthesized buffer's element type is `ulong`, the
// access chains and atomics work on 64-bit pointers, and the SPIR-V
// emitter auto-declares `Int64Atomics` for the resulting module.
//
// Driver compatibility note: this output requires a Vulkan
// implementation that supports `VK_KHR_shader_atomic_int64`. Customers
// targeting drivers that do not (notably MoltenVK on Apple Silicon as
// of MoltenVK 1.4) must compile with
// `-trace-coverage-counter-width 32`; that path is covered by the
// sibling test `coverage-counter-width-opt-down-uint32-spirv.slang`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = tid.x;
}

// SPIR-V must declare both `Int64` (for the storage type) and
// `Int64Atomics` (for the atomic ops). These capabilities are
// auto-emitted by `slang-emit-spirv.cpp` when an atomic op uses a
// 64-bit operand; their presence is the regression guard for the
// uint64 default path.
//CHECK-DAG: OpCapability Int64
//CHECK-DAG: OpCapability Int64Atomics

// The synthesized buffer is named `__slang_coverage` regardless of
// element width.
//CHECK: __slang_coverage

// 64-bit element layout: `OpTypeInt 64 0`, `ArrayStride 8`, and the
// emitted SPIR-V should label the runtime array as `ulong` (SPIR-V's
// disassembly name for `uint64_t`).
//CHECK-DAG: OpTypeInt 64 0
//CHECK-DAG: ArrayStride 8

// Each marker op lowers to an `OpAtomicIAdd` on a 64-bit pointer,
// returning a 64-bit value.
//CHECK: OpAtomicIAdd %ulong
