//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

RWTexture2D<T, 0, format> allocateTexture2D<T : ITexelElement, let format : int>(
    uint width,
    uint height)
{
    uint one = 1;
    return hgbc_asm {
        result : $$RWTexture2D<T, 0, format> = OpAllocateResource
            Texture Texture2D $format DeviceLocal UnorderedAccess $$T
            $width $height $one 1 1 0
            ShaderResource UnorderedAccess CopySource CopyDestination RenderTarget;
    };
}

RWTexture2D<T> allocateTexture2D<T : ITexelElement>(uint width, uint height)
{
    uint one = 1;
    return hgbc_asm {
        result : $$RWTexture2D<T> = OpAllocateResource
            Texture Texture2D $__getHGBCNaturalTextureFormat<T>() DeviceLocal UnorderedAccess $$T
            $width $height $one 1 1 0
            ShaderResource UnorderedAccess CopySource CopyDestination RenderTarget;
    };
}

struct TextureHolder
{
    RWTexture2D<float4> texture;
}

[noinline]
RWTexture2D<float4> passThrough(RWTexture2D<float4> texture)
{
    return texture;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeColor<let format : int>(RWTexture2D<float4, 0, format> texture)
{
    texture[uint2(1, 2)] = float4(0.25, 0.5, 0.75, 1.0);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void checkColor(RWTexture2D<float4> texture, RWStructuredBuffer<uint> result)
{
    float4 actual = texture[uint2(1, 2)];
    result[0] = all(actual == float4(0.25, 0.5, 0.75, 1.0)) ? 1 : 0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeUInt(RWTexture2D<uint> texture)
{
    texture[uint2(0, 1)] = 99;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void copyUInt(RWTexture2D<uint> texture, RWStructuredBuffer<uint> result)
{
    result[0] = texture[uint2(0, 1)];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void checkUnorm(RWTexture2D<float4, 0, 10> texture, RWStructuredBuffer<uint> result)
{
    float4 actual = texture[uint2(1, 2)];
    result[0] = all(uint4(actual * 255.0f + 0.5f) == uint4(64, 127, 191, 255)) ? 1 : 0;
}

[host]
void main()
{
    RWTexture2D<float4> original = allocateTexture2D<float4>(4, 3);
    TextureHolder holder;
    holder.texture = passThrough(original);

    uint width;
    uint height;
    holder.texture.GetDimensions(width, height);
    print(width);
    print(height);

    RWStructuredBuffer<uint> result = RWStructuredBuffer<uint>::alloc(1);
    dispatch(writeColor<0>, uint3(1, 1, 1))(holder.texture);
    dispatch(checkColor, uint3(1, 1, 1))(holder.texture, result);
    print(readBack(result, 0));

    holder.texture = allocateTexture2D<float4>(1, 1);
    holder.texture.GetDimensions(width, height);
    print(width);
    print(height);

    Array<RWTexture2D<float4>, 4> aliases =
        __makeArray(original, holder.texture, original, holder.texture);
    dispatch(checkColor, uint3(1, 1, 1))(aliases[2], result);
    print(readBack(result, 0));

    RWTexture2D<uint> uintTexture = allocateTexture2D<uint>(2, 2);
    dispatch(writeUInt, uint3(1, 1, 1))(uintTexture);
    dispatch(copyUInt, uint3(1, 1, 1))(uintTexture, result);
    print(readBack(result, 0));

    RWTexture2D<float4, 0, 10> unormTexture = allocateTexture2D<float4, 10>(4, 3);
    dispatch(writeColor<10>, uint3(1, 1, 1))(unormTexture);
    dispatch(checkUnorm, uint3(1, 1, 1))(unormTexture, result);
    print(readBack(result, 0));
}

// CHECK: 4
// CHECK-NEXT: 3
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 99
// CHECK-NEXT: 1
