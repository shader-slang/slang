//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute

Texture2D<float4> texture;
static Texture2D<float4> cachedTexture = texture;

export float4 loadCachedTexture()
//CHECK:      ^^^^^^^^^^^^^^^^^ file-scope static resource requires rewritable direct calls
//CHECK:      ^^^^^^^^^^^^^^^^^ function 'loadCachedTexture' accesses a file-scope static resource but may be invoked without an in-module direct call that can receive the resource as an added argument; pass the resource explicitly instead
{
    return cachedTexture.Load(int3(0, 0, 0));
}

[numthreads(1, 1, 1)]
void main()
{}
