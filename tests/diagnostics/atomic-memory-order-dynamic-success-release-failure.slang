// A compareExchange failure order that releases is invalid whatever the success order is, so it
// is diagnosed on every target even when the success order is not a constant. SPIR-V and Metal
// also report the non-constant success order (E41407), so they check their own copy of the call
// under the ENCODED prefix.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl
//DIAGNOSTIC_TEST:SIMPLE(diag=ENCODED):-target spirv -DENCODES_ORDER
//DIAGNOSTIC_TEST:SIMPLE(diag=ENCODED):-target metal -DENCODES_ORDER

RWStructuredBuffer<Atomic<uint>> flag;
RWStructuredBuffer<uint> outb;
uniform int dynamicOrder;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
#ifdef ENCODES_ORDER
    outb[0] = flag[0].compareExchange(0, 1, (MemoryOrder)dynamicOrder, MemoryOrder.Release);
//ENCODED:                           ^ E41407
//ENCODED:                           ^ the memory order of an atomic operation must be a compile-time constant when targeting
//ENCODED:                           ^ E41405
//ENCODED:                           ^ 'MemoryOrder.Release' cannot be used as the memory order of a compareExchange failure
#else
    outb[0] = flag[0].compareExchange(0, 1, (MemoryOrder)dynamicOrder, MemoryOrder.Release);
//CHECK:                             ^ E41405
//CHECK:                             ^ 'MemoryOrder.Release' cannot be used as the memory order of a compareExchange failure
#endif
}
