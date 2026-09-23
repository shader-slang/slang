//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv-asm -entry main -stage compute -validate-ir

Texture2D<float4> texture;
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTexture = texture;

[noinline]
Ptr<Texture2D<float4>> getCachedTexture()
{
    return &cachedTexture;
/*CHECK:
    ^^^^^^ unsupported use of the address of a file-scope static resource
    ^^^^^^ the address of file-scope static resource 'cachedTexture' is retained or observed in a way that cannot be preserved when each function has separate storage
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
//CHECK:            ^ unsupported use of the address of a file-scope static resource
//CHECK:            ^ the address of file-scope static resource 'cachedTexture' is retained or observed in a way that cannot be preserved when each function has separate storage
}

[numthreads(1, 1, 1)]
void main()
{
    Ptr<Texture2D<float4>> cached = getCachedTexture();
    retainCachedTexture(&cachedTexture, cached);
//CHECK:               ^ unsupported use of the address of a file-scope static resource
//CHECK:               ^ the address of file-scope static resource 'cachedTexture' is retained or observed in a way that cannot be preserved when each function has separate storage
    uint address = uint(getCachedTextureAddress());
    output[0] = (*cached).Load(int3(0, 0, 0));
    output[1] = float4(float(address), 0.0, 0.0, 0.0);
}
