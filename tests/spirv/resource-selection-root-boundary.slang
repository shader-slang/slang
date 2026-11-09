//TEST:SIMPLE(filecheck=GLOBAL): -target spirv-asm -stage fragment -entry globalRoot -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLOBAL): -target spirv-asm -stage fragment -entry globalRoot -O1 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PARAM): -target spirv-asm -stage fragment -entry parameterRoot -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PARAM): -target spirv-asm -stage fragment -entry parameterRoot -O1 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=TEXTURE-COPY): -target spirv-asm -stage fragment -entry globalRoot -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SAMPLER-COPY): -target spirv-asm -stage fragment -entry parameterRoot -O0 -validate-ir -validate-ir-detailed

// We sample one texture with a selected sampler and separately copy that texture in assembly.
// The selection pass retains resource roots, so that assembly operation needs no reconstruction
// and must not prevent the sampler selection from being legalized. We also copy a sampler
// parameter of an ordinary function to exercise a retained parameter root.
// We follow the condition into both arms and the declared bindings into their actual samples.
// The separate copy checks run at `-O0`: SPIR-V optimization can remove `OpCopyObject` at `-O1`.
// GLOBAL: %[[UV:[^ ]+]] = OpLoad {{[^ ]+}} %uv{{( |$)}}
// GLOBAL: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// GLOBAL: %[[CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool %[[X]] %float_0_5{{( |$)}}
// GLOBAL: OpSelectionMerge %[[MERGE:[^ ]+]] None
// GLOBAL: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
// GLOBAL: %[[THEN]] = OpLabel
// GLOBAL-DAG: %[[TEXTURE_A:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// GLOBAL-DAG: %[[SAMPLER_A:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// GLOBAL: %[[PAIR_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEXTURE_A]] %[[SAMPLER_A]]{{( |$)}}
// GLOBAL: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_A]]{{( |$)}}
// GLOBAL: OpBranch %[[MERGE]]{{( |$)}}
// GLOBAL: %[[ELSE]] = OpLabel
// GLOBAL-DAG: %[[TEXTURE_B:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// GLOBAL-DAG: %[[SAMPLER_B:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// GLOBAL: %[[PAIR_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEXTURE_B]] %[[SAMPLER_B]]{{( |$)}}
// GLOBAL: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_B]]{{( |$)}}
// GLOBAL: OpBranch %[[MERGE]]{{( |$)}}
// GLOBAL: %[[MERGE]] = OpLabel
// PARAM: %[[SAMPLER:[^ ]+]] = OpTypeSampler
// PARAM: %[[FIRST:[^ ]+]] = OpFunctionParameter %[[SAMPLER]]{{( |$)}}
// PARAM-NEXT: %[[SECOND:[^ ]+]] = OpFunctionParameter %[[SAMPLER]]{{( |$)}}
// PARAM-NEXT: %[[UV:[^ ]+]] = OpFunctionParameter %v2float{{( |$)}}
// PARAM: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// PARAM: %[[CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool %[[X]] %float_0_5{{( |$)}}
// PARAM: OpSelectionMerge %[[MERGE:[^ ]+]] None
// PARAM: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
// PARAM: %[[THEN]] = OpLabel
// PARAM: %[[TEXTURE_A:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// PARAM: %[[PAIR_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEXTURE_A]] %[[FIRST]]{{( |$)}}
// PARAM: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_A]]{{( |$)}}
// PARAM: OpBranch %[[MERGE]]{{( |$)}}
// PARAM: %[[ELSE]] = OpLabel
// PARAM: %[[TEXTURE_B:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// PARAM: %[[PAIR_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEXTURE_B]] %[[SECOND]]{{( |$)}}
// PARAM: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_B]]{{( |$)}}
// PARAM: OpBranch %[[MERGE]]{{( |$)}}
// PARAM: %[[MERGE]] = OpLabel
// TEXTURE-COPY: %[[IMAGE:[^ ]+]] = OpTypeImage
// TEXTURE-COPY: %[[INPUT:[^ ]+]] = OpLoad %[[IMAGE]] %texture{{( |$)}}
// TEXTURE-COPY: %[[COPIED:[^ ]+]] = OpCopyObject %[[IMAGE]] %[[INPUT]]{{( |$)}}
// TEXTURE-COPY: OpImageFetch {{[^ ]+}} %[[COPIED]]{{( |$)}}
// SAMPLER-COPY: %[[SAMPLER:[^ ]+]] = OpTypeSampler
// SAMPLER-COPY: %[[INPUT:[^ ]+]] = OpFunctionParameter %[[SAMPLER]]{{( |$)}}
// SAMPLER-COPY: %[[COPIED:[^ ]+]] = OpCopyObject %[[SAMPLER]] %[[INPUT]]{{( |$)}}
// SAMPLER-COPY-DAG: %[[COMBINED:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEXTURE:[^ ]+]] %[[COPIED]]{{( |$)}}
// SAMPLER-COPY-DAG: %[[TEXTURE]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// SAMPLER-COPY: OpImageSampleExplicitLod {{[^ ]+}} %[[COMBINED]]{{( |$)}}

Texture2D<float4> texture;
SamplerState samplerA;
SamplerState samplerB;

// The selected sample and the independent assembly copy use the same global texture binding.
// Only the selected sample needs rewriting; the final fetch keeps the assembly copy live.
float4 globalRoot(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> copied = spirv_asm
    {
        result : $$Texture2D<float4> = OpCopyObject $texture;
    };
    SamplerState selected = select(uv.x > 0.5, samplerA, samplerB);
    return texture.SampleLevel(selected, uv, 0) + copied.Load(int3(0));
}

// The sampler parameters remain available throughout this function. We select `firstSampler`
// or `secondSampler` for one sample and independently copy `firstSampler` for the other sample.
// Only the sample that uses the selected sampler needs rewriting.
[noinline]
float4 sampleWithParameterRoots(
    SamplerState firstSampler,
    SamplerState secondSampler,
    float2 uv)
{
    SamplerState copied = spirv_asm
    {
        result : $$SamplerState = OpCopyObject $firstSampler;
    };
    SamplerState selected = select(uv.x > 0.5, firstSampler, secondSampler);
    return texture.SampleLevel(selected, uv, 0) + texture.SampleLevel(copied, uv, 1);
}

float4 parameterRoot(float2 uv : TEXCOORD) : SV_Target
{
    return sampleWithParameterRoots(samplerA, samplerB, uv);
}
