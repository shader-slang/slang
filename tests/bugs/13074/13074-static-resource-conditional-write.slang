//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=D3D): -O0 -target dxil -profile ps_6_0 -entry main -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -O0 -target spirv-asm -entry main -stage fragment -validate-ir

//D3D: target cannot represent an opaque value in mutable local storage
//D3D: mutable local storage contains an opaque value of type 'Texture2D', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable

// SPIR-V can lower the conditional update to ordinary state and branch around the later fetch.
// The helper only writes when `choose` is nonzero, so the other path must still fetch `textureA`.
// We follow that condition into each arm and the selected binding into its fetch.
// SPIRV: %[[IMAGE:[^ ]+]] = OpTypeImage
// SPIRV: %[[CHOOSE_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %globalParams %int_0{{( |$)}}
// SPIRV: %[[CHOOSE:[^ ]+]] = OpLoad %uint %[[CHOOSE_PTR]]{{( |$)}}
// SPIRV: %[[NONZERO:[^ ]+]] = OpINotEqual %bool %[[CHOOSE]] %uint_0{{( |$)}}
// SPIRV: %[[KEEP_INITIAL:[^ ]+]] = OpLogicalNot %bool %[[NONZERO]]{{( |$)}}
// SPIRV: OpSelectionMerge %[[MERGE:[^ ]+]] None
// SPIRV: OpBranchConditional %[[KEEP_INITIAL]] %[[INITIAL:[^ ]+]] %[[UPDATED:[^ ]+]]{{( |$)}}
// SPIRV: %[[INITIAL]] = OpLabel
// SPIRV: %[[FIRST:[^ ]+]] = OpLoad %[[IMAGE]] %textureA{{( |$)}}
// SPIRV: OpImageFetch {{[^ ]+}} %[[FIRST]]{{( |$)}}
// SPIRV: OpBranch %[[MERGE]]{{( |$)}}
// SPIRV: %[[UPDATED]] = OpLabel
// SPIRV: %[[SECOND:[^ ]+]] = OpLoad %[[IMAGE]] %textureB{{( |$)}}
// SPIRV: OpImageFetch {{[^ ]+}} %[[SECOND]]{{( |$)}}
// SPIRV: OpBranch %[[MERGE]]{{( |$)}}
// SPIRV: %[[MERGE]] = OpLabel

Texture2D<float4> textureA;
Texture2D<float4> textureB;
uniform uint choose;
static Texture2D<float4> selectedTexture = textureA;

[noinline]
void maybeSelectTexture()
{
    if (choose != 0)
        selectedTexture = textureB;
}

[shader("fragment")]
float4 main() : SV_Target
{
    maybeSelectTexture();
    return selectedTexture.Load(int3(0, 0, 0));
}
