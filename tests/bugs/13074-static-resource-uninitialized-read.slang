//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

// `legalizeResourceGlobalVars` reports the read from the generated entry-point local. The pre-link
// `checkUninitializedGlobals` analysis skips the original global marked for replacement so that the
// same source read produces only one diagnostic.

static Texture2D<float4> selectedTexture;

[shader("fragment")]
float4 main() : SV_Target
{
    return selectedTexture.Load(int3(0, 0, 0));
//CHECK:                       ^ use of uninitialized variable
//CHECK:                       ^ use of uninitialized variable 'selectedTexture'
}
