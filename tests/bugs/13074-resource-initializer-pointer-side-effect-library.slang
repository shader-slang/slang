//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -validate-ir

Texture2D<float4> texture;
RWStructuredBuffer<float> output;
static uint initializationCount = 0;

[noinline]
void increment(Ptr<uint> value)
{
    ++*value;
}

[noinline]
Texture2D<float4> initializeTexture()
{
    increment(&initializationCount);
    return texture;
}

static Texture2D<float4> cachedTexture = initializeTexture();

export uint getInitializationCount()
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ resource-dependent static state is not supported in an independently invoked function
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ function 'getInitializationCount' may be invoked without an entry-point caller and transitively accesses per-invocation resource-dependent static state; pass the required state explicitly instead
{
    return initializationCount;
}

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0)).x;
}
