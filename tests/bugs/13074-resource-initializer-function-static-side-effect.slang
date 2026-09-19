//TEST:SIMPLE: -target hlsl -entry main -stage compute -validate-ir -o 13074-resource-initializer-function-static-side-effect.hlsl

Texture2D<float4> texture;
RWStructuredBuffer<float> output;

[noinline]
void recordInitialization()
{
    static uint initializationCount = 0;
    ++initializationCount;
}

[noinline]
Texture2D<float4> initializeTexture()
{
    recordInitialization();
    return texture;
}

static Texture2D<float4> cachedTexture = initializeTexture();

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0)).x;
}
