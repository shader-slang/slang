//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute

Texture2D<float4> texture;
RWStructuredBuffer<float> output;

export void recordInitialization()
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ resource-dependent static state is not supported in an independently invoked function
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ function 'recordInitialization' may be invoked without an entry-point caller and transitively accesses per-invocation resource-dependent static state; pass the required state explicitly instead
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
