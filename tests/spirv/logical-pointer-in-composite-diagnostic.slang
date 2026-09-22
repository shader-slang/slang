//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// A logical-storage-class pointer read out of a composite that the legalization pass cannot reduce
// to an address -- here the composite is a by-value function return, not a load of an addressable
// variable -- is rejected at emit time rather than silently emitted as an invalid OpCompositeExtract.
// This is the emit-time fallback for shapes the projection-sinking pass cannot rewrite; see
// https://github.com/shader-slang/slang/issues/13206.

struct View
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
}

groupshared float gsA[4];

View makeView()
{
    View v;
    v.data = __getAddress(gsA[0]);
    return v;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: error[E57008]
    // CHECK: cannot be read out of a composite value when targeting SPIR-V
    makeView().data[0] = 1.0f;
}
