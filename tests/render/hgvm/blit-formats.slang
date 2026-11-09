//TEST:INTERPRET(hgvm): -api default

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

void blitTexture2D<TDestination : ITexelElement, TSource : ITexelElement>(
    RWTexture2D<TDestination> destination,
    RWTexture2D<TSource> source)
{
    hgbc_asm {
        OpBlitTexture2D $$RWTexture2D<TDestination> $$RWTexture2D<TSource>
            $destination $source;
    };
}

[host]
void main()
{
    RWTexture2D<float4> floatTexture = allocateTexture2D<float4>(2, 2);
    RWTexture2D<uint4> uintTexture = allocateTexture2D<uint4>(2, 2);
    RWTexture2D<int4> intTexture = allocateTexture2D<int4>(2, 2);

    blitTexture2D(floatTexture, uintTexture);
    blitTexture2D(uintTexture, intTexture);
    blitTexture2D(intTexture, floatTexture);
    // Revisit the first type pair after two other pipelines to exercise the session cache path.
    blitTexture2D(floatTexture, uintTexture);
}
