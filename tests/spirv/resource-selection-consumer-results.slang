//TEST:SIMPLE(filecheck=DISPATCH-INDEX): -target spirv-asm -stage fragment -entry dispatchedIndex -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=DISPATCH-INDEX): -target spirv-asm -stage fragment -entry dispatchedIndex -O1 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SINGLE-INDEX): -target spirv-asm -stage fragment -entry singleRecipeIndex -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SINGLE-INDEX): -target spirv-asm -stage fragment -entry singleRecipeIndex -O1 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=DISPATCH-BOOL): -target spirv-asm -stage fragment -entry dispatchedCondition -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=DISPATCH-BOOL): -target spirv-asm -stage fragment -entry dispatchedCondition -O1 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SINGLE-BOOL): -target spirv-asm -stage fragment -entry singleRecipeCondition -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SINGLE-BOOL): -target spirv-asm -stage fragment -entry singleRecipeCondition -O1 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PROJECTED): -target spirv-asm -stage fragment -entry projectedIndex -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PROJECTED): -target spirv-asm -stage fragment -entry projectedIndex -O1 -validate-ir -validate-ir-detailed

// In the first four entry points, a call consumes a selected texture and returns an ordinary
// value used by a later resource construction or selection. Replacing that call must also update
// the cached selector or index payload used to reconstruct the later combined texture/sampler pair.
// The integer result is an array subscript; the Boolean result selects the sampler.
// `singleRecipeIndex` and `singleRecipeCondition` choose elements of one texture array, so their
// first calls need one reconstruction without dispatch. `dispatchedIndex` and
// `dispatchedCondition` choose separately declared textures and need a texture dispatch.
// The first four entry points use a cbuffer condition and fixed texel coordinate to make their
// resource choices uniform. `projectedIndex` uses a varying sampler choice and texture projection
// to check that the replacement index remains marked as nonuniform.

Texture2D<float4> firstTexture;
Texture2D<float4> secondTexture;
Texture2D<float4> textures[2];
SamplerState samplers[4];
SamplerState firstSampler;
SamplerState secondSampler;
cbuffer Parameters { uint chooseFirst; };

// Return an index in 0..3 derived from the texture's first texel.
[noinline]
uint getSamplerIndex(Texture2D<float4> texture)
{
    // We bound the index inside the callee so its result is used directly as the subscript.
    // A caller-side mask or cast would introduce another instruction whose ordinary operand use
    // could be updated even if the pass's cached index payload still referenced the deleted call.
    return min(asuint(texture.Load(int3(0)).x), 3u);
}

// Return whether the texture's first texel has a positive first component.
[noinline]
bool getSamplerCondition(Texture2D<float4> texture)
{
    // The caller uses the result directly in `select`, so a cached selector aliases the call
    // result itself rather than an instruction that merely uses that result.
    return texture.Load(int3(0)).x > 0;
}

// Return an index in 0..1 derived from an explicit-LOD sample with the given sampler.
[noinline]
uint getTextureIndex(SamplerState sampler)
{
    // Bounding the result inside the callee leaves the call itself as the later array index.
    return min(asuint(firstTexture.SampleLevel(sampler, float2(0), 0).x), 1u);
}

// DISPATCH-INDEX: %[[CHOICE:[^ ]+]] = OpINotEqual %bool{{( |$)}}
// DISPATCH-INDEX: OpSelectionMerge %[[CALL_MERGE:[^ ]+]] None
// DISPATCH-INDEX: OpBranchConditional %[[CHOICE]] %[[CALL_A:[^ ]+]] %[[CALL_B:[^ ]+]]{{( |$)}}
// DISPATCH-INDEX: %[[CALL_A]] = OpLabel
// DISPATCH-INDEX: %[[RESULT_A:[^ ]+]] = OpFunctionCall %uint %[[FN_A:[^ ]+]]{{( |$)}}
// DISPATCH-INDEX: OpStore %[[RESULT_SLOT:[^ ]+]] %[[RESULT_A]]{{( |$)}}
// DISPATCH-INDEX: OpBranch %[[CALL_MERGE]]{{( |$)}}
// DISPATCH-INDEX: %[[CALL_B]] = OpLabel
// DISPATCH-INDEX: %[[RESULT_B:[^ ]+]] = OpFunctionCall %uint %[[FN_B:[^ ]+]]{{( |$)}}
// DISPATCH-INDEX: OpStore %[[RESULT_SLOT]] %[[RESULT_B]]{{( |$)}}
// DISPATCH-INDEX: OpBranch %[[CALL_MERGE]]{{( |$)}}
// DISPATCH-INDEX: %[[CALL_MERGE]] = OpLabel
// DISPATCH-INDEX: OpSelectionMerge %[[SAMPLE_MERGE:[^ ]+]] None
// DISPATCH-INDEX: OpBranchConditional %[[CHOICE]] %[[SAMPLE_A:[^ ]+]] %[[SAMPLE_B:[^ ]+]]{{( |$)}}
// DISPATCH-INDEX: %[[SAMPLE_A]] = OpLabel
// DISPATCH-INDEX: %[[INDEX_A:[^ ]+]] = OpLoad %uint %[[RESULT_SLOT]]{{( |$)}}
// DISPATCH-INDEX: %[[PTR_A:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[INDEX_A]]{{( |$)}}
// DISPATCH-INDEX: %[[IMAGE_A:[^ ]+]] = OpLoad {{[^ ]+}} %firstTexture{{( |$)}}
// DISPATCH-INDEX: %[[SAMPLER_A:[^ ]+]] = OpLoad {{[^ ]+}} %[[PTR_A]]{{( |$)}}
// DISPATCH-INDEX: %[[PAIR_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_A]] %[[SAMPLER_A]]{{( |$)}}
// DISPATCH-INDEX: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_A]]{{( |$)}}
// DISPATCH-INDEX: OpBranch %[[SAMPLE_MERGE]]{{( |$)}}
// DISPATCH-INDEX: %[[SAMPLE_B]] = OpLabel
// DISPATCH-INDEX: %[[INDEX_B:[^ ]+]] = OpLoad %uint %[[RESULT_SLOT]]{{( |$)}}
// DISPATCH-INDEX: %[[PTR_B:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[INDEX_B]]{{( |$)}}
// DISPATCH-INDEX: %[[IMAGE_B:[^ ]+]] = OpLoad {{[^ ]+}} %secondTexture{{( |$)}}
// DISPATCH-INDEX: %[[SAMPLER_B:[^ ]+]] = OpLoad {{[^ ]+}} %[[PTR_B]]{{( |$)}}
// DISPATCH-INDEX: %[[PAIR_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_B]] %[[SAMPLER_B]]{{( |$)}}
// DISPATCH-INDEX: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_B]]{{( |$)}}
// DISPATCH-INDEX: OpBranch %[[SAMPLE_MERGE]]{{( |$)}}
// DISPATCH-INDEX: %[[SAMPLE_MERGE]] = OpLabel
// DISPATCH-INDEX: %[[FN_A]] = OpFunction %uint DontInline{{( |$)}}
// DISPATCH-INDEX: %[[FN_IMAGE_A:[^ ]+]] = OpLoad {{[^ ]+}} %firstTexture{{( |$)}}
// DISPATCH-INDEX: OpImageFetch {{[^ ]+}} %[[FN_IMAGE_A]]{{( |$)}}
// DISPATCH-INDEX: OpFunctionEnd
// DISPATCH-INDEX: %[[FN_B]] = OpFunction %uint DontInline{{( |$)}}
// DISPATCH-INDEX: %[[FN_IMAGE_B:[^ ]+]] = OpLoad {{[^ ]+}} %secondTexture{{( |$)}}
// DISPATCH-INDEX: OpImageFetch {{[^ ]+}} %[[FN_IMAGE_B]]{{( |$)}}
float4 dispatchedIndex(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(chooseFirst != 0, firstTexture, secondTexture);
    uint index = getSamplerIndex(texture);
    Sampler2D<float4> pair = __makeCombinedTextureSampler(texture, samplers[index]);
    return pair.SampleLevel(uv, 0);
}

// SINGLE-INDEX: %[[TEXTURE_INDEX:[^ ]+]] = OpSelect %uint {{[^ ]+}} %uint_0 %uint_1{{( |$)}}
// SINGLE-INDEX: %[[INDEX:[^ ]+]] = OpFunctionCall %uint %[[FN:[^ ]+]] %[[TEXTURE_INDEX]]{{( |$)}}
// SINGLE-INDEX: %[[TEXTURE_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %textures %[[TEXTURE_INDEX]]{{( |$)}}
// SINGLE-INDEX: %[[SAMPLER_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[INDEX]]{{( |$)}}
// SINGLE-INDEX: %[[IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %[[TEXTURE_PTR]]{{( |$)}}
// SINGLE-INDEX: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[SAMPLER_PTR]]{{( |$)}}
// SINGLE-INDEX: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE]] %[[SAMPLER]]{{( |$)}}
// SINGLE-INDEX: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}
// SINGLE-INDEX: %[[FN]] = OpFunction %uint DontInline{{( |$)}}
// SINGLE-INDEX: %[[INPUT_INDEX:[^ ]+]] = OpFunctionParameter %uint{{( |$)}}
// SINGLE-INDEX: %[[INPUT_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %textures %[[INPUT_INDEX]]{{( |$)}}
// SINGLE-INDEX: %[[INPUT_IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %[[INPUT_PTR]]{{( |$)}}
// SINGLE-INDEX: OpImageFetch {{[^ ]+}} %[[INPUT_IMAGE]]{{( |$)}}
float4 singleRecipeIndex(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(chooseFirst != 0, textures[0], textures[1]);
    uint index = getSamplerIndex(texture);
    Sampler2D<float4> pair = __makeCombinedTextureSampler(texture, samplers[index]);
    return pair.SampleLevel(uv, 0);
}

// DISPATCH-BOOL: %[[CHOICE:[^ ]+]] = OpINotEqual %bool{{( |$)}}
// DISPATCH-BOOL: OpSelectionMerge %[[CALL_MERGE:[^ ]+]] None
// DISPATCH-BOOL: OpBranchConditional %[[CHOICE]] %[[CALL_A:[^ ]+]] %[[CALL_B:[^ ]+]]{{( |$)}}
// DISPATCH-BOOL: %[[CALL_A]] = OpLabel
// DISPATCH-BOOL: %[[RESULT_A:[^ ]+]] = OpFunctionCall %bool %[[FN_A:[^ ]+]]{{( |$)}}
// DISPATCH-BOOL: OpStore %[[RESULT_SLOT:[^ ]+]] %[[RESULT_A]]{{( |$)}}
// DISPATCH-BOOL: OpBranch %[[CALL_MERGE]]{{( |$)}}
// DISPATCH-BOOL: %[[CALL_B]] = OpLabel
// DISPATCH-BOOL: %[[RESULT_B:[^ ]+]] = OpFunctionCall %bool %[[FN_B:[^ ]+]]{{( |$)}}
// DISPATCH-BOOL: OpStore %[[RESULT_SLOT]] %[[RESULT_B]]{{( |$)}}
// DISPATCH-BOOL: OpBranch %[[CALL_MERGE]]{{( |$)}}
// DISPATCH-BOOL: %[[CALL_MERGE]] = OpLabel
// DISPATCH-BOOL: %[[RESULT:[^ ]+]] = OpLoad %bool %[[RESULT_SLOT]]{{( |$)}}
// DISPATCH-BOOL: %[[NOT_RESULT:[^ ]+]] = OpLogicalNot %bool %[[RESULT]]{{( |$)}}
// DISPATCH-BOOL: %[[TAG_A:[^ ]+]] = OpSelect %uint %[[NOT_RESULT]] %uint_1 %uint_0{{( |$)}}
// DISPATCH-BOOL: %[[TAG_B:[^ ]+]] = OpSelect %uint %[[NOT_RESULT]] %uint_3 %uint_2{{( |$)}}
// DISPATCH-BOOL: %[[NOT_CHOICE:[^ ]+]] = OpLogicalNot %bool %[[CHOICE]]{{( |$)}}
// DISPATCH-BOOL: %[[TAG:[^ ]+]] = OpSelect %uint %[[NOT_CHOICE]] %[[TAG_B]] %[[TAG_A]]{{( |$)}}
// DISPATCH-BOOL: OpSelectionMerge %[[SAMPLE_MERGE:[^ ]+]] None
// DISPATCH-BOOL: OpSwitch %[[TAG]] %[[BB:[^ ]+]] 0 %[[AA:[^ ]+]] 1 %[[AB:[^ ]+]] 2 %[[BA:[^ ]+]]{{( |$)}}
// DISPATCH-BOOL: %[[BB]] = OpLabel
// DISPATCH-BOOL: %[[IMAGE_BB:[^ ]+]] = OpLoad {{[^ ]+}} %secondTexture{{( |$)}}
// DISPATCH-BOOL: %[[SAMPLER_BB:[^ ]+]] = OpLoad {{[^ ]+}} %secondSampler{{( |$)}}
// DISPATCH-BOOL: %[[PAIR_BB:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_BB]] %[[SAMPLER_BB]]{{( |$)}}
// DISPATCH-BOOL: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_BB]]{{( |$)}}
// DISPATCH-BOOL: OpBranch %[[SAMPLE_MERGE]]{{( |$)}}
// DISPATCH-BOOL: %[[AA]] = OpLabel
// DISPATCH-BOOL: %[[IMAGE_AA:[^ ]+]] = OpLoad {{[^ ]+}} %firstTexture{{( |$)}}
// DISPATCH-BOOL: %[[SAMPLER_AA:[^ ]+]] = OpLoad {{[^ ]+}} %firstSampler{{( |$)}}
// DISPATCH-BOOL: %[[PAIR_AA:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_AA]] %[[SAMPLER_AA]]{{( |$)}}
// DISPATCH-BOOL: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_AA]]{{( |$)}}
// DISPATCH-BOOL: OpBranch %[[SAMPLE_MERGE]]{{( |$)}}
// DISPATCH-BOOL: %[[AB]] = OpLabel
// DISPATCH-BOOL: %[[IMAGE_AB:[^ ]+]] = OpLoad {{[^ ]+}} %firstTexture{{( |$)}}
// DISPATCH-BOOL: %[[SAMPLER_AB:[^ ]+]] = OpLoad {{[^ ]+}} %secondSampler{{( |$)}}
// DISPATCH-BOOL: %[[PAIR_AB:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_AB]] %[[SAMPLER_AB]]{{( |$)}}
// DISPATCH-BOOL: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_AB]]{{( |$)}}
// DISPATCH-BOOL: OpBranch %[[SAMPLE_MERGE]]{{( |$)}}
// DISPATCH-BOOL: %[[BA]] = OpLabel
// DISPATCH-BOOL: %[[IMAGE_BA:[^ ]+]] = OpLoad {{[^ ]+}} %secondTexture{{( |$)}}
// DISPATCH-BOOL: %[[SAMPLER_BA:[^ ]+]] = OpLoad {{[^ ]+}} %firstSampler{{( |$)}}
// DISPATCH-BOOL: %[[PAIR_BA:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_BA]] %[[SAMPLER_BA]]{{( |$)}}
// DISPATCH-BOOL: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_BA]]{{( |$)}}
// DISPATCH-BOOL: OpBranch %[[SAMPLE_MERGE]]{{( |$)}}
// DISPATCH-BOOL: %[[SAMPLE_MERGE]] = OpLabel
// DISPATCH-BOOL: %[[FN_A]] = OpFunction %bool DontInline{{( |$)}}
// DISPATCH-BOOL: %[[FN_IMAGE_A:[^ ]+]] = OpLoad {{[^ ]+}} %firstTexture{{( |$)}}
// DISPATCH-BOOL: OpImageFetch {{[^ ]+}} %[[FN_IMAGE_A]]{{( |$)}}
// DISPATCH-BOOL: OpFunctionEnd
// DISPATCH-BOOL: %[[FN_B]] = OpFunction %bool DontInline{{( |$)}}
// DISPATCH-BOOL: %[[FN_IMAGE_B:[^ ]+]] = OpLoad {{[^ ]+}} %secondTexture{{( |$)}}
// DISPATCH-BOOL: OpImageFetch {{[^ ]+}} %[[FN_IMAGE_B]]{{( |$)}}
float4 dispatchedCondition(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(chooseFirst != 0, firstTexture, secondTexture);
    bool condition = getSamplerCondition(texture);
    SamplerState sampler = select(condition, firstSampler, secondSampler);
    return __makeCombinedTextureSampler(texture, sampler).SampleLevel(uv, 0);
}

// SINGLE-BOOL: %[[TEXTURE_INDEX:[^ ]+]] = OpSelect %int {{[^ ]+}} %int_0 %int_1{{( |$)}}
// SINGLE-BOOL: %[[RESULT:[^ ]+]] = OpFunctionCall %bool %[[FN:[^ ]+]] %[[TEXTURE_INDEX]]{{( |$)}}
// SINGLE-BOOL: OpSelectionMerge %[[MERGE:[^ ]+]] None
// SINGLE-BOOL: OpBranchConditional %[[RESULT]] %[[FIRST:[^ ]+]] %[[SECOND:[^ ]+]]{{( |$)}}
// SINGLE-BOOL: %[[FIRST]] = OpLabel
// SINGLE-BOOL: %[[IMAGE_PTR_A:[^ ]+]] = OpAccessChain {{[^ ]+}} %textures %[[TEXTURE_INDEX]]{{( |$)}}
// SINGLE-BOOL: %[[IMAGE_A:[^ ]+]] = OpLoad {{[^ ]+}} %[[IMAGE_PTR_A]]{{( |$)}}
// SINGLE-BOOL: %[[SAMPLER_A:[^ ]+]] = OpLoad {{[^ ]+}} %firstSampler{{( |$)}}
// SINGLE-BOOL: %[[PAIR_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_A]] %[[SAMPLER_A]]{{( |$)}}
// SINGLE-BOOL: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_A]]{{( |$)}}
// SINGLE-BOOL: OpBranch %[[MERGE]]{{( |$)}}
// SINGLE-BOOL: %[[SECOND]] = OpLabel
// SINGLE-BOOL: %[[IMAGE_PTR_B:[^ ]+]] = OpAccessChain {{[^ ]+}} %textures %[[TEXTURE_INDEX]]{{( |$)}}
// SINGLE-BOOL: %[[IMAGE_B:[^ ]+]] = OpLoad {{[^ ]+}} %[[IMAGE_PTR_B]]{{( |$)}}
// SINGLE-BOOL: %[[SAMPLER_B:[^ ]+]] = OpLoad {{[^ ]+}} %secondSampler{{( |$)}}
// SINGLE-BOOL: %[[PAIR_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_B]] %[[SAMPLER_B]]{{( |$)}}
// SINGLE-BOOL: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_B]]{{( |$)}}
// SINGLE-BOOL: OpBranch %[[MERGE]]{{( |$)}}
// SINGLE-BOOL: %[[MERGE]] = OpLabel
// SINGLE-BOOL: %[[FN]] = OpFunction %bool DontInline{{( |$)}}
// SINGLE-BOOL: %[[INPUT_INDEX:[^ ]+]] = OpFunctionParameter %int{{( |$)}}
// SINGLE-BOOL: %[[INPUT_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %textures %[[INPUT_INDEX]]{{( |$)}}
// SINGLE-BOOL: %[[INPUT_IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %[[INPUT_PTR]]{{( |$)}}
// SINGLE-BOOL: OpImageFetch {{[^ ]+}} %[[INPUT_IMAGE]]{{( |$)}}
float4 singleRecipeCondition(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(chooseFirst != 0, textures[0], textures[1]);
    bool condition = getSamplerCondition(texture);
    SamplerState sampler = select(condition, firstSampler, secondSampler);
    return __makeCombinedTextureSampler(texture, sampler).SampleLevel(uv, 0);
}

// Both sampler alternatives in `pair` contain the same `textures[index]`. Extracting its texture
// merges equal index payloads, which fold to the earlier call result itself. The replacement
// result must keep the nonuniform-index classification needed by the final array access.
// PROJECTED: OpDecorate %[[INDEX:[^ ]+]] NonUniform{{( |$)}}
// PROJECTED: %[[CHOICE:[^ ]+]] = OpFOrdGreaterThan %bool{{( |$)}}
// PROJECTED: OpSelectionMerge %[[CALL_MERGE:[^ ]+]] None
// PROJECTED: OpBranchConditional %[[CHOICE]] %[[CALL_A:[^ ]+]] %[[CALL_B:[^ ]+]]{{( |$)}}
// PROJECTED: %[[CALL_A]] = OpLabel
// PROJECTED: %[[ARG_A:[^ ]+]] = OpLoad {{[^ ]+}} %firstSampler{{( |$)}}
// PROJECTED: %[[RESULT_A:[^ ]+]] = OpFunctionCall %uint %[[FN_A:[^ ]+]] %[[ARG_A]]{{( |$)}}
// PROJECTED: OpStore %[[RESULT_SLOT:[^ ]+]] %[[RESULT_A]]{{( |$)}}
// PROJECTED: OpBranch %[[CALL_MERGE]]{{( |$)}}
// PROJECTED: %[[CALL_B]] = OpLabel
// PROJECTED: %[[ARG_B:[^ ]+]] = OpLoad {{[^ ]+}} %secondSampler{{( |$)}}
// PROJECTED: %[[RESULT_B:[^ ]+]] = OpFunctionCall %uint %[[FN_A]] %[[ARG_B]]{{( |$)}}
// PROJECTED: OpStore %[[RESULT_SLOT]] %[[RESULT_B]]{{( |$)}}
// PROJECTED: OpBranch %[[CALL_MERGE]]{{( |$)}}
// PROJECTED: %[[CALL_MERGE]] = OpLabel
// PROJECTED: %[[INDEX]] = OpLoad %uint %[[RESULT_SLOT]]{{( |$)}}
// PROJECTED: %[[IMAGE_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %textures %[[INDEX]]{{( |$)}}
// PROJECTED: %[[IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %[[IMAGE_PTR]]{{( |$)}}
// PROJECTED: OpImageFetch {{[^ ]+}} %[[IMAGE]]{{( |$)}}
float4 projectedIndex(float2 uv : TEXCOORD) : SV_Target
{
    SamplerState sampler = select(uv.x > 0.5, firstSampler, secondSampler);
    uint index = getTextureIndex(sampler);
    Sampler2D<float4> pair = __makeCombinedTextureSampler(textures[index], sampler);
    return pair.Load(int3(0));
}
