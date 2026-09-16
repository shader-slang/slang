//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute

Texture2D<float4> texture;
Texture2D<float4> secondTexture;
static Texture2D<float4> cachedTexture = texture;
static Texture2D<float4> secondCachedTexture = secondTexture;
static float4 cachedValue = cachedTexture.Load(int3(0, 0, 0));
static float4 secondCachedValue = secondCachedTexture.Load(int3(0, 0, 0));

export float4 unrelatedExport(float4 value)
{
    return value;
}

[noinline]
float4 readCachedValue()
{
    return cachedValue;
}

export float4 loadBothCachedValues()
//CHECK:      ^^^^^^^^^^^^^^^^^^^^ resource-dependent static state is not supported in an independently invoked function
//CHECK:      ^^^^^^^^^^^^^^^^^^^^ function 'loadBothCachedValues' may be invoked without an entry-point caller and transitively accesses per-invocation resource-dependent static state; pass the required state explicitly instead
{
    return cachedValue + secondCachedValue;
}

export float4 loadCachedValue()
//CHECK:      ^^^^^^^^^^^^^^^ resource-dependent static state is not supported in an independently invoked function
//CHECK:      ^^^^^^^^^^^^^^^ function 'loadCachedValue' may be invoked without an entry-point caller and transitively accesses per-invocation resource-dependent static state; pass the required state explicitly instead
{
    return readCachedValue();
}

[numthreads(1, 1, 1)]
void main()
{}
