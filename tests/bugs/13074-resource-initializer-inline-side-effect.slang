// Moving `cachedTexture`'s initializer into `main` would also move the increment. The increment
// modifies observable state, so we reject this initializer instead of changing when it executes.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -validate-ir -warnings-disable 41024

static uint initializationCount = 0;
Texture2D<float4> texture;
RWStructuredBuffer<float4> output;

static Texture2D<float4> cachedTexture = (++initializationCount, texture);
//CHECK:                 ^^^^^^^^^^^^^ cannot prove that a mutable resource initializer can be moved safely
//CHECK:                 ^^^^^^^^^^^^^ Slang cannot prove that moving the initializer for the mutable resource variable 'cachedTexture' to each entry point preserves observable behavior. The initializer may have externally observable side effects or may read preexisting mutable storage, resource contents, or non-resource data from an explicit parameter group; initialize the resource explicitly in an entry point instead

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0)) + initializationCount;
}
