// A helper that cannot return does not need to preserve the value supplied by its caller. The
// caller's uninitialized local must therefore not be copied into the helper.
//
//TEST:SIMPLE: -target hlsl -entry main -stage compute -validate-ir -o 13074-static-resource-no-return-writer.hlsl

Texture2D<float4> inputTexture;
static Texture2D<float4> cachedTexture;

[noinline]
void assignAndLoopForever()
{
    cachedTexture = inputTexture;
    for (;;)
    {
    }
}

[numthreads(1, 1, 1)]
void main()
{
    assignAndLoopForever();
}
