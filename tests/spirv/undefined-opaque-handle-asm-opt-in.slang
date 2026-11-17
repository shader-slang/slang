//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -O0
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -minimum-slang-optimization

// Check that specializing the undefined argument preserves the block's annotation,
// while the other call still fetches the defined texture.
// CHECK: OpImageFetch
// CHECK-NOT: OpImageFetch

Texture2D<float4> texture;
RWStructuredBuffer<float4> outputBuffer;
cbuffer Settings
{
    bool useUndefined;
};

[noinline]
float4 fetch(Texture2D<float4> image, int2 coordinates)
{
    int lod = 0;
    return spirv_asm [__assumeValidResourceOperands]
    {
        result:$$float4 = OpImageFetch $image $coordinates Lod $lod;
    };
}

[numthreads(1, 1, 1)]
void main()
{
    Texture2D<float4> missing;
    float4 value = fetch(texture, int2(0));
    if (useUndefined)
        value = fetch(missing, int2(0));
    outputBuffer[0] = value;
}
