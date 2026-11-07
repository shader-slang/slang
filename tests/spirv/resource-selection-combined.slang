//TEST:SIMPLE(filecheck=PRECOMBINED): -target spirv-asm -stage fragment -entry precombined -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PRECOMBINED): -target spirv-asm -stage fragment -entry precombined -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry precombined -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=MIXED): -target spirv-asm -stage fragment -entry mixed -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=MIXED): -target spirv-asm -stage fragment -entry mixed -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=PROJECTED): -target spirv-asm -stage fragment -entry projected -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PROJECTED): -target spirv-asm -stage fragment -entry projected -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=FETCH-COUNT): -target spirv-asm -stage fragment -entry projected -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=FETCH-COUNT): -target spirv-asm -stage fragment -entry projected -O1 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=EXTRACT): -target spirv-asm -stage fragment -entry constructedSampler -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=EXTRACT): -target spirv-asm -stage fragment -entry constructedSampler -validate-ir -validate-ir-detailed -O1

// A combined texture and sampler can come from a single declared binding or from a texture
// and a sampler combined in the program. When `select` chooses between those forms, we must retain
// the complete texture/sampler value for each alternative; a single combined binding cannot be
// replaced with imaginary separate bindings.
// The explicit pair constructor is the SPIR-V library intrinsic. GLSL library methods construct
// texture/sampler pairs through textual intrinsics, which are covered by the other test files.

Sampler2D<float4> combinedA;
Sampler2D<float4> combinedB;
Texture2D<float4> texture;
SamplerState sampler;
Texture2D<float4> textureArray[4];
SamplerState otherSampler;
cbuffer Parameters { uint index; uint otherIndex; };

// The two branch arms must sample their respective combined bindings. Capturing both the
// branch labels and loaded values prevents unused binding loads from satisfying these checks.
// PRECOMBINED: %[[UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// PRECOMBINED: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// PRECOMBINED: %[[CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool %[[X]] %float_0_5{{( |$)}}
// PRECOMBINED: OpSelectionMerge %[[MERGE:[^ ]+]] None
// PRECOMBINED: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
// PRECOMBINED: %[[THEN]] = OpLabel
// PRECOMBINED: %[[FIRST:[^ ]+]] = OpLoad {{[^ ]+}} %combinedA{{( |$)}}
// PRECOMBINED: OpImageSampleExplicitLod {{[^ ]+}} %[[FIRST]]{{( |$)}}
// PRECOMBINED: OpBranch %[[MERGE]]{{( |$)}}
// PRECOMBINED: %[[ELSE]] = OpLabel
// PRECOMBINED: %[[SECOND:[^ ]+]] = OpLoad {{[^ ]+}} %combinedB{{( |$)}}
// PRECOMBINED: OpImageSampleExplicitLod {{[^ ]+}} %[[SECOND]]{{( |$)}}
// PRECOMBINED: OpBranch %[[MERGE]]{{( |$)}}
// PRECOMBINED: %[[MERGE]] = OpLabel
// GLSL-DAG: uniform sampler2D [[FIRST_BINDING:combinedA[^; ]*]];
// GLSL-DAG: uniform sampler2D [[SECOND_BINDING:combinedB[^; ]*]];
// GLSL-DAG: in vec2 [[UV:uv[^; ]*]];
// GLSL: if(([[UV]].x) > 0.5)
// GLSL: textureLod({{[(]*}}[[FIRST_BINDING]]{{[)]*}},
// GLSL: else
// GLSL: textureLod({{[(]*}}[[SECOND_BINDING]]{{[)]*}},
float4 precombined(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = select(uv.x > 0.5, combinedA, combinedB);
    return selected.SampleLevel(uv, 0);
}

// One arm samples the existing combined binding. The other must construct a pair from the
// separately declared texture and sampler, then use that pair for its sample.
// MIXED: %[[UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// MIXED: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// MIXED: %[[CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool %[[X]] %float_0_5{{( |$)}}
// MIXED: OpSelectionMerge %[[MERGE:[^ ]+]] None
// MIXED: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
// MIXED: %[[THEN]] = OpLabel
// MIXED: %[[COMBINED:[^ ]+]] = OpLoad {{[^ ]+}} %combinedA{{( |$)}}
// MIXED: OpImageSampleExplicitLod {{[^ ]+}} %[[COMBINED]]{{( |$)}}
// MIXED: OpBranch %[[MERGE]]{{( |$)}}
// MIXED: %[[ELSE]] = OpLabel
// MIXED: %[[TEXTURE:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// MIXED: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %sampler{{( |$)}}
// MIXED: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEXTURE]] %[[SAMPLER]]{{( |$)}}
// MIXED: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}
// MIXED: OpBranch %[[MERGE]]{{( |$)}}
// MIXED: %[[MERGE]] = OpLabel
float4 mixed(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = select(uv.x > 0.5,
        combinedA, __makeCombinedTextureSampler(texture, sampler));
    return selected.SampleLevel(uv, 0);
}

// `Load` extracts the texture from a combined texture and sampler before fetching a texel.
// Both incoming textures use `textureArray`, so discarding the sampler must merge the two
// texture indices rather than discard one index together with the sampler distinction.
// We follow the selected index through the access chain and image load to the fetch, so an
// unused access to the selected element cannot satisfy the image-fetch check.
// PROJECTED: %[[UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// PROJECTED: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// PROJECTED: %[[CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool %[[X]] %float_0_5{{( |$)}}
// PROJECTED: %[[FIRST_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Parameters %int_0{{( |$)}}
// PROJECTED: %[[FIRST:[^ ]+]] = OpLoad %uint %[[FIRST_POINTER]]{{( |$)}}
// PROJECTED: %[[SECOND_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Parameters %int_1{{( |$)}}
// PROJECTED: %[[SECOND:[^ ]+]] = OpLoad %uint %[[SECOND_POINTER]]{{( |$)}}
// PROJECTED: %[[FIRST_INDEX:[^ ]+]] = OpSelect %uint %[[CONDITION]] %[[FIRST]] %uint_0{{( |$)}}
// PROJECTED: %[[SECOND_INDEX:[^ ]+]] = OpSelect %uint %[[CONDITION]] %uint_0 %[[SECOND]]{{( |$)}}
// PROJECTED: %[[INVERSE:[^ ]+]] = OpLogicalNot %bool %[[CONDITION]]{{( |$)}}
// PROJECTED: %[[INDEX:[^ ]+]] = OpSelect %uint %[[INVERSE]] %[[SECOND_INDEX]] %[[FIRST_INDEX]]{{( |$)}}
// PROJECTED: %[[IMAGE_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %textureArray %[[INDEX]]{{( |$)}}
// PROJECTED: %[[IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %[[IMAGE_POINTER]]{{( |$)}}
// PROJECTED: OpImageFetch {{[^ ]+}} %[[IMAGE]]{{( |$)}}
// FETCH-COUNT: OpImageFetch
// FETCH-COUNT-NOT: OpImageFetch
float4 projected(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = select(uv.x > 0.5,
        __makeCombinedTextureSampler(textureArray[index], sampler),
        __makeCombinedTextureSampler(textureArray[otherIndex], otherSampler));
    return selected.Load(int3(0));
}

// Extracting the sampler from a pair constructed in the program is valid because both sampler
// arguments already exist as separate resources. The constructed pairs in `selected` must reduce
// to their actual sampler arguments. Each conditional sample must then combine `texture` with
// the corresponding sampler.
// EXTRACT: %[[UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// EXTRACT: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// EXTRACT: %[[CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool %[[X]] %float_0_5{{( |$)}}
// EXTRACT: OpSelectionMerge %[[MERGE:[^ ]+]] None
// EXTRACT: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
// EXTRACT: %[[THEN]] = OpLabel
// EXTRACT: %[[TEX_A:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// EXTRACT: %[[SAMPLER_A:[^ ]+]] = OpLoad {{[^ ]+}} %sampler{{( |$)}}
// EXTRACT: %[[PAIR_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEX_A]] %[[SAMPLER_A]]{{( |$)}}
// EXTRACT: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_A]]{{( |$)}}
// EXTRACT: OpBranch %[[MERGE]]{{( |$)}}
// EXTRACT: %[[ELSE]] = OpLabel
// EXTRACT: %[[TEX_B:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// EXTRACT: %[[SAMPLER_B:[^ ]+]] = OpLoad {{[^ ]+}} %otherSampler{{( |$)}}
// EXTRACT: %[[PAIR_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEX_B]] %[[SAMPLER_B]]{{( |$)}}
// EXTRACT: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_B]]{{( |$)}}
// EXTRACT: OpBranch %[[MERGE]]{{( |$)}}
// EXTRACT: %[[MERGE]] = OpLabel
float4 constructedSampler(float2 uv : TEXCOORD) : SV_Target
{
    Sampler2D<float4> selected = select(uv.x > 0.5,
        __makeCombinedTextureSampler(texture, sampler),
        __makeCombinedTextureSampler(texture, otherSampler));
    return texture.SampleLevel(selected.__getSampler(), uv, 0);
}
