//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute

Texture2D<float4> texture;
static Texture2D<float4> cachedTexture = texture;

export float4 loadCachedTexture()
//CHECK:      ^^^^^^^^^^^^^^^^^ resource declared static at file or namespace scope is unavailable to a function whose invocations cannot all be rewritten
//CHECK:      ^^^^^^^^^^^^^^^^^ function 'loadCachedTexture' may be invoked without a direct call that Slang can rewrite, so Slang cannot automatically pass the resource that it accesses; pass the resource explicitly instead
{
    return cachedTexture.Load(int3(0, 0, 0));
}

[numthreads(1, 1, 1)]
void main()
{}
