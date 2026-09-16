//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv-asm -entry main -stage compute -validate-ir

Texture2D<float4> texture;
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTexture = texture;

[noinline]
Ptr<Texture2D<float4>> getCachedTexture()
{
    return &cachedTexture;
/*CHECK:
    ^^^^^^ address of file-scope static resource cannot escape
    ^^^^^^ the address of file-scope static resource 'cachedTexture' escapes the function; pass the resource value or a caller-owned reference explicitly instead
*/
}

[numthreads(1, 1, 1)]
void main()
{
    Ptr<Texture2D<float4>> cached = getCachedTexture();
    output[0] = (*cached).Load(int3(0, 0, 0));
}
