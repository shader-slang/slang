// Reading buffer contents observes mutable resource state. Moving this initializer could observe a
// different value at `textureIndices[0]` and therefore select a different texture.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -validate-ir

Texture2D<float4> textures[2];
RWStructuredBuffer<uint> textureIndices;

static Texture2D<float4> cachedTexture = textures[textureIndices[0]];
//CHECK:                 ^^^^^^^^^^^^^ initializer for a resource declared static at file or namespace scope cannot be moved safely
//CHECK:                 ^^^^^^^^^^^^^ Slang cannot prove that moving the initializer for resource 'cachedTexture', which is declared static at file or namespace scope, to each entry point preserves observable behavior. The initializer may have externally observable side effects or may read preexisting mutable storage, resource contents, or non-resource data from a source-declared parameter group; initialize the resource explicitly in an entry point instead

RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
