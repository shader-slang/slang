//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute

Texture2D<float4> texture;
static Texture2D<float4> cachedTexture = texture;
//CHECK:                 ^^^^^^^^^^^^^ resource-dependent static state is not supported in an independently invoked function
//CHECK:                 ^^^^^^^^^^^^^ static variable 'cachedTexture' depends on per-invocation resource state and cannot be used by function 'loadCachedTexture', which may be invoked without an entry-point caller; pass the required state explicitly instead

export float4 loadCachedTexture()
{
    return cachedTexture.Load(int3(0, 0, 0));
}

[numthreads(1, 1, 1)]
void main()
{}
