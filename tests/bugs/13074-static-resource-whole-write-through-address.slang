//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -o 13074-static-resource-whole-write-through-address.cpp

Texture2D<float4> inputTexture;
RWStructuredBuffer<float4> output;

static Texture2D<float4> cachedTexture;

[noinline]
void initializeCachedTexture()
{
    *__getAddress(cachedTexture) = inputTexture;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    initializeCachedTexture();
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
