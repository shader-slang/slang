//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -llvm -compute
// Resource types are lowered into concrete types in the LLVM target.
// This means that we should be able to get a concrete value for their size with
// `sizeof` and that we should be able to default-initialize and bitCast them.

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct Foo
{
    // Unfortunately, we can't just use the = {} default initialization here, as
    // these are magic structs with no default constructors.
    Texture2D<float4> tex = __default<Texture2D<float4>>();
    ByteAddressBuffer buf = __default<ByteAddressBuffer>();
    RaytracingAccelerationStructure as = __default<RaytracingAccelerationStructure>();
};

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 8
    outputBuffer[0] = sizeof(Texture2D<float4>);
    // CHECK: 10
    outputBuffer[1] = sizeof(StructuredBuffer<int>);
    // CHECK: 10
    outputBuffer[2] = sizeof(RWStructuredBuffer<float2>);

    Foo f = {};
    // CHECK: 20
    outputBuffer[3] = sizeof(f);

    // Let's make the texture read-write now.
    RWTexture2D<float4> tex = bit_cast<RWTexture2D<float4>>(f.tex);
    // Let's take a look at the pointer, it should still be nullptr.
    void* texPtr = bit_cast<void*>(tex);

    // CHECK: 1
    outputBuffer[4] = texPtr == nullptr ? 1 : 0;
}
