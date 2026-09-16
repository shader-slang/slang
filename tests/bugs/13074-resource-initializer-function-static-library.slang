//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute

Texture2D<float4> texture;
RWStructuredBuffer<float> output;

export void recordInitialization()
{
    static uint initializationCount = 0;
//CHECK:        ^^^^^^^^^^^^^^^^^^^ resource-dependent static state is not supported in an independently invoked function
//CHECK:        ^^^^^^^^^^^^^^^^^^^ static variable 'initializationCount' depends on per-invocation resource state and cannot be used by function 'recordInitialization', which may be invoked without an entry-point caller; pass the required state explicitly instead
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
