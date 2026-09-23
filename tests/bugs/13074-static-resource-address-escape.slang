//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv-asm -entry main -stage compute -validate-ir

Texture2D<float4> texture;
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTexture = texture;

[noinline]
Ptr<Texture2D<float4>> getCachedTexture()
{
    return &cachedTexture;
/*CHECK:
    ^^^^^^ address use may observe the identity of storage for a resource declared static at file or namespace scope
    ^^^^^^ the program may retain the address of resource 'cachedTexture', which is declared static at file or namespace scope, or observe its storage identity; replacing the variable with function-local storage might change that behavior
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
//CHECK:            ^ address use may observe the identity of storage for a resource declared static at file or namespace scope
//CHECK:            ^ the program may retain the address of resource 'cachedTexture', which is declared static at file or namespace scope, or observe its storage identity; replacing the variable with function-local storage might change that behavior
}

[numthreads(1, 1, 1)]
void main()
{
    Ptr<Texture2D<float4>> cached = getCachedTexture();
    retainCachedTexture(&cachedTexture, cached);
//CHECK:               ^ address use may observe the identity of storage for a resource declared static at file or namespace scope
//CHECK:               ^ the program may retain the address of resource 'cachedTexture', which is declared static at file or namespace scope, or observe its storage identity; replacing the variable with function-local storage might change that behavior
    uint address = uint(getCachedTextureAddress());
    output[0] = (*cached).Load(int3(0, 0, 0));
    output[1] = float4(float(address), 0.0, 0.0, 0.0);
}
