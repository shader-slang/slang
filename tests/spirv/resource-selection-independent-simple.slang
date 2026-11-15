//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -DMANUAL -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -DMANUAL -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=NO-TAG): -target spirv-asm -stage fragment -entry main -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=NO-TAG): -target spirv-asm -stage fragment -entry main -DMANUAL -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=NO-TAG): -target spirv-asm -stage fragment -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=NO-TAG): -target spirv-asm -stage fragment -entry main -DMANUAL -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -DMANUAL -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -DMANUAL -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=COUNTS): -target spirv-asm -stage fragment -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=COUNTS): -target spirv-asm -stage fragment -entry main -DMANUAL -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=COUNTS): -target spirv-asm -stage fragment -entry main -O1 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=COUNTS): -target spirv-asm -stage fragment -entry main -DMANUAL -O1 -validate-ir -validate-ir-detailed

// We compare a selected sampler with the equivalent hand-written conditional.
// Both forms must branch on the original boolean and sample only the selected resource.
// The separate `NO-TAG` checks cover the complete module and reject switches, integer selects,
// and cancelling Boolean negations in the simple case.
// The `COUNTS` checks cover the complete module and require exactly one conditional branch,
// two samples, and two unconditional branches, in the same order as the hand-written code.
// SPIRV: %[[UV:[^ ]+]] = OpLoad {{[^ ]+}} %uv{{( |$)}}
// SPIRV: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// SPIRV: %[[CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool %[[X]] %float_0_5{{( |$)}}
// SPIRV: OpSelectionMerge %[[MERGE:[^ ]+]] None
// SPIRV: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
// SPIRV: %[[THEN]] = OpLabel
// SPIRV-DAG: %[[TEXTURE_A:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// SPIRV-DAG: %[[SAMPLER_A:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// SPIRV: %[[PAIR_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEXTURE_A]] %[[SAMPLER_A]]{{( |$)}}
// SPIRV: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_A]]{{( |$)}}
// SPIRV: OpBranch %[[MERGE]]{{( |$)}}
// SPIRV: %[[ELSE]] = OpLabel
// SPIRV-DAG: %[[TEXTURE_B:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// SPIRV-DAG: %[[SAMPLER_B:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// SPIRV: %[[PAIR_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEXTURE_B]] %[[SAMPLER_B]]{{( |$)}}
// SPIRV: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_B]]{{( |$)}}
// SPIRV: OpBranch %[[MERGE]]{{( |$)}}
// SPIRV: %[[MERGE]] = OpLabel
// NO-TAG-NOT: OpSwitch
// NO-TAG-NOT: OpSelect %int
// NO-TAG-NOT: OpSelect %uint
// NO-TAG-NOT: OpLogicalNot
// COUNTS-NOT: Op{{(BranchConditional|ImageSampleExplicitLod|Branch )}}
// COUNTS: OpBranchConditional
// COUNTS-NOT: Op{{(BranchConditional|ImageSampleExplicitLod|Branch )}}
// COUNTS: OpImageSampleExplicitLod
// COUNTS-NOT: Op{{(BranchConditional|ImageSampleExplicitLod|Branch )}}
// COUNTS: OpBranch %
// COUNTS-NOT: Op{{(BranchConditional|ImageSampleExplicitLod|Branch )}}
// COUNTS: OpImageSampleExplicitLod
// COUNTS-NOT: Op{{(BranchConditional|ImageSampleExplicitLod|Branch )}}
// COUNTS: OpBranch %
// COUNTS-NOT: Op{{(BranchConditional|ImageSampleExplicitLod|Branch )}}
// GLSL-DAG: uniform texture2D [[TEXTURE:texture[^; ]*]];
// GLSL-DAG: uniform sampler [[SAMPLER_A:samplerA[^; ]*]];
// GLSL-DAG: uniform sampler [[SAMPLER_B:samplerB[^; ]*]];
// GLSL-DAG: in vec2 [[UV:uv[^; ]*]];
// GLSL-NOT: textureLod
// GLSL: if(([[UV]].x) > 0.5)
// GLSL-NOT: if(
// GLSL-NOT: textureLod
// GLSL: textureLod(sampler2D([[TEXTURE]],[[SAMPLER_A]]),
// GLSL-NOT: textureLod
// GLSL-NOT: if(
// GLSL: else
// GLSL-NOT: if(
// GLSL-NOT: textureLod
// GLSL: textureLod(sampler2D([[TEXTURE]],[[SAMPLER_B]]),
// GLSL-NOT: textureLod
// GLSL-NOT: if(

Texture2D<float4> texture;
SamplerState samplerA;
SamplerState samplerB;

float4 main(float2 uv : TEXCOORD) : SV_Target
{
#ifdef MANUAL
    float4 result;
    if (uv.x > 0.5)
        result = texture.SampleLevel(samplerA, uv, 0);
    else
        result = texture.SampleLevel(samplerB, uv, 0);
    return result;
#else
    SamplerState selected = select(uv.x > 0.5, samplerA, samplerB);
    return texture.SampleLevel(selected, uv, 0);
#endif
}
