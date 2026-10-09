//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv-asm -entry main -stage compute -validate-ir

Texture2D<float4> texture;
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTexture = texture;

[noinline]
Ptr<Texture2D<float4>> getCachedTexture()
{
    return &cachedTexture;
/*CHECK:
    ^^^^^^ address use may observe the identity of mutable resource storage
    ^^^^^^ the program may retain the address of mutable resource variable 'cachedTexture' or observe its storage identity; replacing the variable with function-local storage might change that behavior
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
//CHECK:            ^ address use may observe the identity of mutable resource storage
//CHECK:            ^ the program may retain the address of mutable resource variable 'cachedTexture' or observe its storage identity; replacing the variable with function-local storage might change that behavior
}

[numthreads(1, 1, 1)]
void main()
{
    // Inline SPIR-V receives the address of `cachedTexture`. The `legalizeResourceGlobalVars` pass
    // does not interpret the assembly deeply enough to prove that replacing `cachedTexture` with
    // function-local addresses preserves the assembly's behavior. The pass therefore diagnoses
    // this use instead of replacing `cachedTexture`.
    spirv_asm
/*CHECK:
    ^^^^^^^^^ address use may observe the identity of mutable resource storage
    ^^^^^^^^^ the program may retain the address of mutable resource variable 'cachedTexture' or observe its storage identity; replacing the variable with function-local storage might change that behavior
*/
    {
        OpStore &cachedTexture $texture;
    };

    Ptr<Texture2D<float4>> cached = getCachedTexture();
    retainCachedTexture(&cachedTexture, cached);
//CHECK:               ^ address use may observe the identity of mutable resource storage
//CHECK:               ^ the program may retain the address of mutable resource variable 'cachedTexture' or observe its storage identity; replacing the variable with function-local storage might change that behavior
    uint address = uint(getCachedTextureAddress());
    output[0] = (*cached).Load(int3(0, 0, 0));
    output[1] = float4(float(address), 0.0, 0.0, 0.0);
}
