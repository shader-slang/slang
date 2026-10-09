// SPIR-V and Metal encode the memory order of an atomic operation in the generated code, so they
// require it to be a compile-time constant `MemoryOrder` value. The other targets ignore the
// order and accept the same code. The rows cover each operand position that holds an order: load
// and increment, store, exchange and add, and both orders of compareExchange.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal
//TEST:SIMPLE(filecheck=CHECK_OK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_OK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_OK):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_OK):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<Atomic<uint>> flag;
RWStructuredBuffer<uint> outb;
uniform int dynamicOrder;

// Without inlining, `order` is a parameter of the function rather than a constant, even though
// every caller passes a constant.
uint loadWithOrder(MemoryOrder order)
{
    return flag[0].load(order);
//CHECK:               ^ E41407
//CHECK:               ^ the memory order of an atomic operation must be a compile-time constant when targeting
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = flag[0].load((MemoryOrder)dynamicOrder);
//CHECK:                  ^ E41407
//CHECK:                  ^ the memory order of an atomic operation must be a compile-time constant when targeting
    outb[1] = loadWithOrder(MemoryOrder.Acquire);
    outb[2] = flag[0].load((MemoryOrder)7);
//CHECK:                  ^ E41408
//CHECK:                  ^ 7 is not a valid MemoryOrder value
    outb[3] = flag[0].add(1, (MemoryOrder)dynamicOrder);
//CHECK:                 ^ E41407
//CHECK:                 ^ the memory order of an atomic operation must be a compile-time constant when targeting
    outb[4] = flag[0].increment((MemoryOrder)dynamicOrder);
//CHECK:                       ^ E41407
//CHECK:                       ^ the memory order of an atomic operation must be a compile-time constant when targeting
    outb[5] = flag[0].exchange(1, (MemoryOrder)dynamicOrder);
//CHECK:                      ^ E41407
//CHECK:                      ^ the memory order of an atomic operation must be a compile-time constant when targeting
    flag[1].store(1, (MemoryOrder)dynamicOrder);
//CHECK:         ^ E41407
//CHECK:         ^ the memory order of an atomic operation must be a compile-time constant when targeting
    outb[6] = flag[0].compareExchange(0, 1, MemoryOrder.AcquireRelease, (MemoryOrder)dynamicOrder);
//CHECK:                             ^ E41407
//CHECK:                             ^ the memory order of an atomic operation must be a compile-time constant when targeting
    outb[7] = flag[0].compareExchange(0, 1, MemoryOrder.AcquireRelease, (MemoryOrder)7);
//CHECK:                             ^ E41408
//CHECK:                             ^ 7 is not a valid MemoryOrder value
    outb[8] = flag[0].compareExchange(0, 1, (MemoryOrder)7, MemoryOrder.Acquire);
//CHECK:                             ^ E41408
//CHECK:                             ^ 7 is not a valid MemoryOrder value
    outb[9] = flag[0].compareExchange(0, 1, (MemoryOrder)7, (MemoryOrder)7);
//CHECK:                             ^ E41408
//CHECK:                             ^ 7 is not a valid MemoryOrder value
}

// CHECK_OK: result code = 0
