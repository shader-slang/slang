// Companion regression for https://github.com/shader-slang/slang/issues/11731.
//
// gh-11731.slang covers `__atomic_add` (the `OpAtomicIAdd` path through
// `emitMemorySemanticMask` -- gate-1, the Memory Semantics storage-class mask).
// This file covers `Atomic<T>` load/store/exchange/compareExchange. Of those,
// load/store/exchange additionally route through `isAtomicableAddressSpace`
// (gate-2): when targeting SPIR-V < 1.4 an SSBO pointer is in the legacy
// `Uniform` storage class (BufferBlock model), and if that gate does not
// recognize the `Uniform` spelling load/store degrade to non-atomic
// `OpLoad`/`OpStore` while exchange aborts the compiler. compareExchange does
// NOT gate on atomicability -- it emits its atomic opcode unconditionally -- so
// it exercises only gate-1; it is included here for the dual-mask coverage
// described at its CHECK below.
//
// The `-profile spirv_1_3` target forces the pre-1.4 `Uniform`/`BufferBlock`
// SSBO model, and every op must still emit its atomic opcode with the
// `UniformMemory` (0x40) storage-class bit set in the Memory Semantics operand.
//TEST:SIMPLE(filecheck=CHECK):-profile spirv_1_3 -target spirv

RWStructuredBuffer<Atomic<uint>> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // The SSBO is in the `Uniform` storage class (BufferBlock), not
    // `StorageBuffer`, because the target is SPIR-V < 1.4.
    // CHECK: OpDecorate {{.*}} BufferBlock
    // CHECK-NOT: OpTypePointer{{.*}}StorageBuffer
    // Acquire(0x2) | UniformMemory(0x40) == 0x42 == 66
    // CHECK: OpAtomicLoad {{.*}} %uint_66
    // Release(0x4) | UniformMemory(0x40) == 0x44 == 68
    // CHECK: OpAtomicStore {{.*}} %uint_68
    // AcquireRelease(0x8) | UniformMemory(0x40) == 0x48 == 72
    // CHECK: OpAtomicExchange {{.*}} %uint_72
    // compareExchange is the only atomic that emits TWO Memory Semantics masks
    // (equal = successOrder, unequal = failOrder); both must carry UniformMemory
    // so a regression re-breaking only one mask cannot slip the single-mask
    // cases above. success AcquireRelease(0x8)|0x40 == 72, fail Acquire(0x2)|0x40 == 66
    // CHECK: OpAtomicCompareExchange {{.*}} %uint_72 %uint_66
    let v = buf[0].load(MemoryOrder.Acquire);
    buf[0].store(v, MemoryOrder.Release);
    let old = buf[0].exchange(v, MemoryOrder.AcquireRelease);
    buf[1].compareExchange(old, v, MemoryOrder.AcquireRelease, MemoryOrder.Acquire);
}
