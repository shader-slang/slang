//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry main -stage compute -validate-ir

Texture2D<float4> inputTexture;
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTexture = inputTexture;
static Texture2D<float4>* retainedAddress;

[noinline]
void retainAddress(__ref Texture2D<float4> value)
{
    retainedAddress = &value;
}

[noinline]
void exposeCachedTexture()
{
    retainAddress(cachedTexture);
//CHECK:         ^ address use may observe the identity of storage for a resource declared static at file or namespace scope
//CHECK:         ^ the program may retain the address of resource 'cachedTexture', which is declared static at file or namespace scope, or observe its storage identity; replacing the variable with function-local storage might change that behavior
}

[numthreads(1, 1, 1)]
void main()
{
    exposeCachedTexture();
    output[0] = (*retainedAddress).Load(int3(0, 0, 0));
}
