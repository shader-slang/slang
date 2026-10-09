// A compareExchange failure order that releases is invalid whatever the success order is, so it
// is diagnosed on every target even when the success order is not a constant. SPIR-V and Metal
// also report the non-constant success order (E41407, covered by
// atomic-memory-order-not-constant.slang), so their directives are non-exhaustive.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target metal

RWStructuredBuffer<Atomic<uint>> flag;
RWStructuredBuffer<uint> outb;
uniform int dynamicOrder;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = flag[0].compareExchange(0, 1, (MemoryOrder)dynamicOrder, MemoryOrder.Release);
//CHECK:                             ^ E41405
//CHECK:                             ^ 'MemoryOrder.Release' cannot be used as the memory order of a compareExchange failure
}
