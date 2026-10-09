// Memory orders are validated on the final IR, so these programs compile on every target: an
// order held in a `static const` global folds to its constant, and an atomic in a branch that is
// removed as dead code is never checked. The -g2 directives check that neither depends on the
// debug-info level.
//
// An atomic intrinsic declared without an order (ORDERLESS_ATOMIC) has no order to check. It runs
// on HLSL only, because the SPIR-V and Metal emitters assume every order is present.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -g2
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute -g2
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute -DORDERLESS_ATOMIC

RWStructuredBuffer<Atomic<uint>> flag;
RWStructuredBuffer<uint> outb;
uniform int dynamicOrder;

static const MemoryOrder globalOrder = MemoryOrder.Acquire;
static const bool useDynamicOrder = false;

#ifdef ORDERLESS_ATOMIC
__intrinsic_op(atomicAdd)
uint atomicAddWithoutOrder(__ref uint dst, uint value);
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = flag[0].load(globalOrder);

    if (useDynamicOrder)
        outb[1] = flag[0].load((MemoryOrder)dynamicOrder);
    else
        outb[1] = flag[0].load(MemoryOrder.Acquire);

#ifdef ORDERLESS_ATOMIC
    atomicAddWithoutOrder(outb[2], 1u);
#endif
}

// CHECK: result code = 0
