// Companion regression test for shader-slang/slang#13107: non-POD rejection.
//
// `isSimpleConstantType` was widened to accept POD structs. It must still reject
// a struct that contains a resource / opaque field, so such a struct is never
// accepted as a device-global constant table. The fold/no-fold signal is the
// synthesized constructor: in the POD case it is folded to a `makeStruct` and
// DCE'd (see the sibling tests, which assert it is absent); here it must survive
// (not folded), proving the struct was rejected by the simple-constant gate.
//
// (This emits identically to master; the point is to lock the rejection so the
// widened classifier cannot regress while the POD matrix stays green. The
// separate module-scope `__device__` intermediates the resource path emits are
// pre-existing behavior unrelated to this change and are intentionally not
// asserted on here.)

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none

struct Bindful
{
    uint id;
    DescriptorHandle<RWStructuredBuffer<uint> > buf;
}

// On CUDA a descriptor handle is a 64-bit value, so the handles are made from `uint64_t`; the
// `uint2` constructor is not available there, and the entry point reads this initializer.
static const Bindful gItems[2] = {
    {1u, (DescriptorHandle<RWStructuredBuffer<uint> >)uint64_t(3)},
    {2u, (DescriptorHandle<RWStructuredBuffer<uint> >)uint64_t(5)},
};

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = gItems[tid.x].id;
}

// The synthesized constructor is NOT folded away (the struct is not a simple
// constant), which is how the widened POD classifier is kept safe.
// CUDA: Bindful{{.*}}init
