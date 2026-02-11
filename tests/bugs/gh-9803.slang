//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

// This should compile without "unexpected: needed a known integer value" internal error

public struct DescriptorHandleWAR<T : IOpaqueDescriptor>
{
    public uint2 handle;
};

__generic< T : ITexelElement, Shape : __ITextureShape, let isArray : int, let isMS : int, let sampleCount : int, let access : int, let isShadow : int, let isCombined : int, let format : int>
extension _Texture<T, Shape, isArray, isMS, sampleCount, access, isShadow, isCombined, format> : __IDynamicResourceCastable<__DynamicResourceKind.General>
{
    __implicit_conversion(10)
    [ForceInline]
    __init(DescriptorHandleWAR<This> war)
    {
        DescriptorHandle<This> handle = DescriptorHandle<This>(war.handle);
        return getDescriptorFromHandle(handle);
    }
}

[numthreads(1,1,1)]
void computeMain() {}

// CHECK-NOT: needed a known integer value
// CHECK: void computeMain
