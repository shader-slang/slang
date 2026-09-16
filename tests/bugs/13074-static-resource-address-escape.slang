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

[noinline]
void retainCachedTexture(
    Ptr<Texture2D<float4>> value,
    out Ptr<Texture2D<float4>> retainedValue)
{
    retainedValue = value;
}

[noinline]
uintptr_t getCachedTextureAddress()
{
    return uintptr_t(&cachedTexture);
//CHECK:            ^ address of file-scope static resource cannot escape
//CHECK:            ^ the address of file-scope static resource 'cachedTexture' escapes the function; pass the resource value or a caller-owned reference explicitly instead
}

[numthreads(1, 1, 1)]
void main()
{
    Ptr<Texture2D<float4>> cached = getCachedTexture();
    retainCachedTexture(&cachedTexture, cached);
//CHECK:               ^ address of file-scope static resource cannot escape
//CHECK:               ^ the address of file-scope static resource 'cachedTexture' escapes the function; pass the resource value or a caller-owned reference explicitly instead
    uint address = uint(getCachedTextureAddress());
    output[0] = (*cached).Load(int3(0, 0, 0));
    output[1] = float4(float(address), 0.0, 0.0, 0.0);
}
