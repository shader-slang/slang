//TEST:INTERPRET(filecheck=CHECK):

#define CHECK(x) if (!(x)) return false

DescriptorAccess getAccess<T : IOpaqueDescriptor>()
{
    return T.descriptorAccess;
}

bool test()
{
    CHECK(getAccess<Texture2D>() == DescriptorAccess.Read);
    CHECK(getAccess<SamplerState>() == DescriptorAccess.Unknown);
    // RasterizerOrdered *buffers* report ReadWrite (per hlsl.meta.slang's
    // kDynamicResourceCastableTypes), while RasterizerOrdered *textures* report
    // DescriptorAccess.RasterizerOrdered. Whether buffers should also use the distinct
    // RasterizerOrdered access level is deferred to #12488; this pins current behavior.
    CHECK(getAccess<RasterizerOrderedStructuredBuffer<int>>() == DescriptorAccess.ReadWrite);
    CHECK(getAccess<RWStructuredBuffer<int>>() == DescriptorAccess.ReadWrite);
    CHECK(getAccess<AppendStructuredBuffer<int>>() == DescriptorAccess.ReadWrite);
    CHECK(getAccess<ConsumeStructuredBuffer<int>>() == DescriptorAccess.ReadWrite);
    CHECK(getAccess<StructuredBuffer<int>>() == DescriptorAccess.Read);
    CHECK(getAccess<ConstantBuffer<int>>() == DescriptorAccess.Read);
    CHECK(getAccess<RWByteAddressBuffer>() == DescriptorAccess.ReadWrite);
    CHECK(getAccess<ByteAddressBuffer>() == DescriptorAccess.Read);
    CHECK(getAccess<RaytracingAccelerationStructure>() == DescriptorAccess.Read);
    CHECK(getAccess<WTexture2DArray>() == DescriptorAccess.Write);
    return true;
}

void main()
{
    // CHECK: pass
    if (test())
        printf("pass\n");
    else
        printf("fail\n");
}