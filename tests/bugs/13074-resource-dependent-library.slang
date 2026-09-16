//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute

Texture2D<float4> texture;
static Texture2D<float4> cachedTexture = texture;
static float4 cachedValue = cachedTexture.Load(int3(0, 0, 0));
//CHECK:      ^^^^^^^^^^^ resource-dependent static state is not supported in an independently invoked function
//CHECK:      ^^^^^^^^^^^ static variable 'cachedValue' depends on per-invocation resource state and cannot be used by function 'loadCachedValue', which may be invoked without an entry-point caller; pass the required state explicitly instead

export float4 unrelatedExport(float4 value)
{
    return value;
}

[noinline]
float4 readCachedValue()
{
    return cachedValue;
}

export float4 loadCachedValue()
{
    return readCachedValue();
}

[numthreads(1, 1, 1)]
void main()
{}
