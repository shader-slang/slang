// Memory orders are validated on the final IR, so these programs compile on every target: an
// order held in a `static` global folds to its constant, and an atomic in a branch that is removed
// as dead code is never checked. An atomic intrinsic declared without an order has nothing to
// check, and a Metal texture atomic takes no order, so its order may be dynamic.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL_TEXTURE):-target metal -entry computeMain -stage compute -DTEXTURE_ATOMIC
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute -DORDERLESS_ATOMIC
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute -DORDERLESS_ATOMIC

RWStructuredBuffer<Atomic<uint>> flag;
RWStructuredBuffer<uint> outb;
uniform int dynamicOrder;

static MemoryOrder globalOrder = MemoryOrder.Acquire;
static const bool useDynamicOrder = false;

#ifdef ORDERLESS_ATOMIC
__intrinsic_op(atomicAdd)
uint atomicAddWithoutOrder(__ref uint dst, uint value);
#endif

#ifdef TEXTURE_ATOMIC
RWTexture2D<uint> tex;

uint addToTexture(MemoryOrder order)
{
    return __atomic_add(tex[uint2(0, 0)], 1u, order);
}
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

#ifdef TEXTURE_ATOMIC
    outb[3] = addToTexture(MemoryOrder.Acquire);
    outb[4] = __atomic_add(tex[uint2(0, 0)], 1u, (MemoryOrder)dynamicOrder);
#endif
}

// CHECK: result code = 0
// METAL_TEXTURE: result code = 0
// METAL_TEXTURE: atomic_fetch_add
