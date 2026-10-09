// A memory order that is invalid for its atomic operation is diagnosed on every target, including
// those that drop the order: a load cannot release, a store cannot acquire, and a compareExchange
// failure order cannot release and needs a success order at least as strong. The check runs after
// specialization and inlining, so it also sees orders forwarded through a generic value parameter
// or a [ForceInline] helper.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda

RWStructuredBuffer<Atomic<uint>> flag;
RWStructuredBuffer<uint> outb;

uint loadWithGenericOrder<let order : MemoryOrder>()
{
    return flag[0].load(order);
//CHECK:               ^ E41405
//CHECK:               ^ 'MemoryOrder.Release' cannot be used as the memory order of an atomic load
}

[ForceInline]
void storeWithOrder(MemoryOrder order)
{
    flag[2].store(1, order);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = flag[0].load(MemoryOrder.Release);
//CHECK:                  ^ E41405
//CHECK:                  ^ 'MemoryOrder.Release' cannot be used as the memory order of an atomic load
    outb[1] = flag[0].load(MemoryOrder.AcquireRelease);
//CHECK:                  ^ E41405
//CHECK:                  ^ 'MemoryOrder.AcquireRelease' cannot be used as the memory order of an atomic load
    flag[1].store(1, MemoryOrder.Acquire);
//CHECK:         ^ E41405
//CHECK:         ^ 'MemoryOrder.Acquire' cannot be used as the memory order of an atomic store
    flag[1].store(2, MemoryOrder.AcquireRelease);
//CHECK:         ^ E41405
//CHECK:         ^ 'MemoryOrder.AcquireRelease' cannot be used as the memory order of an atomic store
    outb[2] = flag[0].compareExchange(0, 1, MemoryOrder.AcquireRelease, MemoryOrder.Release);
//CHECK:                             ^ E41405
//CHECK:                             ^ 'MemoryOrder.Release' cannot be used as the memory order of a compareExchange failure
    outb[3] = flag[0].compareExchange(0, 1, MemoryOrder.AcquireRelease, MemoryOrder.AcquireRelease);
//CHECK:                             ^ E41405
//CHECK:                             ^ 'MemoryOrder.AcquireRelease' cannot be used as the memory order of a compareExchange failure
    outb[4] = flag[0].compareExchange(0, 1, MemoryOrder.Relaxed, MemoryOrder.Acquire);
//CHECK:                             ^ E41406
//CHECK:                             ^ success memory order 'MemoryOrder.Relaxed' must be at least as strong as failure memory order 'MemoryOrder.Acquire'
    outb[5] = flag[0].compareExchange(0, 1, MemoryOrder.Release, MemoryOrder.Acquire);
//CHECK:                             ^ E41406
//CHECK:                             ^ success memory order 'MemoryOrder.Release' must be at least as strong as failure memory order 'MemoryOrder.Acquire'
    outb[6] = flag[0].compareExchange(0, 1, MemoryOrder.AcquireRelease, MemoryOrder.SeqCst);
//CHECK:                             ^ E41406
//CHECK:                             ^ success memory order 'MemoryOrder.AcquireRelease' must be at least as strong as failure memory order 'MemoryOrder.SeqCst'

    outb[7] = loadWithGenericOrder<MemoryOrder.Release>();
    storeWithOrder(MemoryOrder.Acquire);
//CHECK:          ^ E41405
//CHECK:          ^ 'MemoryOrder.Acquire' cannot be used as the memory order of an atomic store

    // Valid orders, including the defaults, are accepted.
    outb[8] = flag[0].load();
    outb[9] = flag[0].load(MemoryOrder.Acquire);
    outb[10] = flag[0].load(MemoryOrder.SeqCst);
    flag[1].store(3);
    flag[1].store(4, MemoryOrder.Release);
    outb[11] = flag[0].exchange(1, MemoryOrder.Acquire);
    outb[12] = flag[0].compareExchange(0, 1);
    outb[13] = flag[0].compareExchange(0, 1, MemoryOrder.AcquireRelease, MemoryOrder.Acquire);
    outb[14] = flag[0].compareExchange(0, 1, MemoryOrder.Release, MemoryOrder.Relaxed);
    outb[15] = flag[0].compareExchange(0, 1, MemoryOrder.Acquire, MemoryOrder.Acquire);
    outb[16] = flag[0].compareExchange(0, 1, MemoryOrder.SeqCst, MemoryOrder.SeqCst);
    outb[17] = flag[0].increment(MemoryOrder.Release);
    outb[18] = flag[0].add(1, MemoryOrder.AcquireRelease);
    outb[19] = loadWithGenericOrder<MemoryOrder.Acquire>();
    storeWithOrder(MemoryOrder.Release);
}
