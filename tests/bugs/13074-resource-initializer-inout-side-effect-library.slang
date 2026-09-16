//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -validate-ir

Texture2D<float4> texture;
RWStructuredBuffer<float> output;
static uint initializationCount = 0;
//CHECK:    ^^^^^^^^^^^^^^^^^^^ resource-dependent static state is not supported in an independently invoked function
//CHECK:    ^^^^^^^^^^^^^^^^^^^ static variable 'initializationCount' depends on per-invocation resource state and cannot be used by function 'getInitializationCount', which may be invoked without an entry-point caller; pass the required state explicitly instead

[noinline]
void increment(inout uint value)
{
    ++value;
}

[noinline]
Texture2D<float4> initializeTexture()
{
    increment(initializationCount);
    return texture;
}

static Texture2D<float4> cachedTexture = initializeTexture();

export uint getInitializationCount()
{
    return initializationCount;
}

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0)).x;
}
