// Moving `cachedTexture`'s initializer into `main` would also move the increment. The increment
// modifies observable state, so we reject this initializer instead of changing when it executes.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -validate-ir -warnings-disable 41024

static uint initializationCount = 0;
Texture2D<float4> texture;
RWStructuredBuffer<float4> output;

static Texture2D<float4> cachedTexture = (++initializationCount, texture);
//CHECK:                 ^^^^^^^^^^^^^ unsupported file-scope static resource initializer
//CHECK:                 ^^^^^^^^^^^^^ initializer for file-scope static resource 'cachedTexture' has a side effect or reads mutable state; initialize the resource explicitly in an entry point instead

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0)) + initializationCount;
}
