// Generic assembly supplies the emitted implementation of `chooseTexture` instead of the function
// body. The initializer analysis cannot inspect that assembly's effects, so it must reject moving
// this call to each entry point.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment -validate-ir

Texture2D<float4> inputTexture;

[noinline]
Texture2D<float4> chooseTexture()
{
    __intrinsic_asm "$0", inputTexture;
}

static Texture2D<float4> cachedTexture = chooseTexture();
//CHECK:                 ^^^^^^^^^^^^^ initializer for a resource declared static at file or namespace scope cannot be moved safely
//CHECK:                 ^^^^^^^^^^^^^ Slang cannot prove that moving the initializer for resource 'cachedTexture', which is declared static at file or namespace scope, to each entry point preserves observable behavior. The initializer may have externally observable side effects or may read preexisting mutable storage, resource contents, or non-resource data from a source-declared parameter group; initialize the resource explicitly in an entry point instead

[shader("fragment")]
float4 main() : SV_Target
{
    return cachedTexture.Load(int3(0, 0, 0));
}
