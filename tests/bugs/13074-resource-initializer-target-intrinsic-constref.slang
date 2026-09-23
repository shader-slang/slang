// A `__constref` argument lowers to an address of the resource value. An applicable target
// intrinsic has no IR body that proves whether it reads through that address, so moving this
// initializer is unsafe even though the intrinsic is marked `__readNone`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment -validate-ir

Texture2D<float4> inputTexture;

[__readNone]
__target_intrinsic(hlsl, "readAndReturnTexture($0)")
Texture2D<float4> inspectTexture(__constref Texture2D<float4> value);

static Texture2D<float4> cachedTexture = inspectTexture(inputTexture);
//CHECK:                 ^^^^^^^^^^^^^ initializer for a resource declared static at file or namespace scope cannot be moved safely
//CHECK:                 ^^^^^^^^^^^^^ Slang cannot prove that moving the initializer for resource 'cachedTexture', which is declared static at file or namespace scope, to each entry point preserves observable behavior. The initializer may have externally observable side effects or may read preexisting mutable storage, resource contents, or non-resource data from a source-declared parameter group; initialize the resource explicitly in an entry point instead

[shader("fragment")]
float4 main() : SV_Target
{
    return cachedTexture.Load(int3(0, 0, 0));
}
