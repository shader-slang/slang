//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute

Texture2D<float4> texture;
static Texture2D<float4> cachedTexture = texture;

export float4 loadCachedTexture()
//CHECK:      ^^^^^^^^^^^^^^^^^ resource-dependent static state is not supported in an independently invoked function
//CHECK:      ^^^^^^^^^^^^^^^^^ function 'loadCachedTexture' may be invoked without an entry-point caller and transitively accesses per-invocation resource-dependent static state; pass the required state explicitly instead
{
    return cachedTexture.Load(int3(0, 0, 0));
}

[numthreads(1, 1, 1)]
void main()
{}
