// Memory orders that are valid for their atomic operation still reach the SPIR-V memory semantics
// operand, including orders forwarded through a generic value parameter or a [ForceInline] helper.
// The semantics are the order bit OR'd with UniformMemory (64): Acquire = 66, Release = 68,
// AcquireRelease = 72, Relaxed = 0.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<Atomic<uint>> flag;
RWStructuredBuffer<uint> outb;

uint loadWithGenericOrder<let order : MemoryOrder>()
{
    return flag[0].load(order);
}

[ForceInline]
void storeWithOrder(MemoryOrder order)
{
    flag[1].store(1, order);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK-DAG: OpAtomicLoad {{.*}} %uint_66
    outb[0] = loadWithGenericOrder<MemoryOrder.Acquire>();
    // CHECK-DAG: OpAtomicStore {{.*}} %uint_68
    storeWithOrder(MemoryOrder.Release);
    // CHECK-DAG: OpAtomicCompareExchange {{.*}} %uint_72 %uint_66
    outb[1] = flag[0].compareExchange(0, 1, MemoryOrder.AcquireRelease, MemoryOrder.Acquire);
    // CHECK-DAG: OpAtomicCompareExchange {{.*}} %uint_68 %uint_0
    outb[2] = flag[0].compareExchange(0, 1, MemoryOrder.Release, MemoryOrder.Relaxed);
}
