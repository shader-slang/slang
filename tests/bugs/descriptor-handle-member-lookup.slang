//TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile sm_6_6 -entry computeMain -stage compute

// Test that DescriptorHandle<T> instance members and extension methods
// are visible through dot syntax, not shadowed by the implicit dereference to T.

extension<T> DescriptorHandle<T>
    where T : IOpaqueDescriptor
{
    bool isNull()
    {
        return uint2(this).x == 0;
    }
}

RWStructuredBuffer<uint> output;
uniform Texture2D<float4>.Handle handle;

// CHECK: void computeMain
// CHECK: isNull_0
// CHECK: output{{.*}}= {{.*}}1
// CHECK: output{{.*}}= {{.*}}2

[numthreads(1,1,1)]
void computeMain()
{
    if (handle.isNull())
        output[0] = 1;

    if (handle.equals(handle))
        output[0] = 2;
}
