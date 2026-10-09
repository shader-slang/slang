// A texture atomic encodes its memory order on SPIR-V, so a non-constant order is an error there.
// Metal's texture atomic functions take no order, so the same code compiles on Metal.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute

RWTexture2D<uint> tex;
RWStructuredBuffer<uint> outb;
uniform int dynamicOrder;

// Without inlining, `order` is a parameter of the function rather than a constant.
uint addToTexture(MemoryOrder order)
{
    return __atomic_add(tex[uint2(0, 0)], 1u, order);
//CHECK:               ^ E41407
//CHECK:               ^ the memory order of an atomic operation must be a compile-time constant when targeting
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = addToTexture(MemoryOrder.Acquire);
    outb[1] = __atomic_add(tex[uint2(0, 0)], 1u, (MemoryOrder)dynamicOrder);
//CHECK:                  ^ E41407
//CHECK:                  ^ the memory order of an atomic operation must be a compile-time constant when targeting
}

// METAL: result code = 0
// METAL: atomic_fetch_add
