//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CONDITION0): -target spirv-asm -stage fragment -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CONDITION1): -target spirv-asm -stage fragment -entry main -O1 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL_STORAGE): -target glsl -stage fragment -entry main -validate-ir -validate-ir-detailed

// This is the example from issue #14101. Each conditional selects an independently declared
// binding. We require conditional branches whose `SampleLevel` and `Load` operations refer directly
// to those bindings, without a function-local texture, sampler, or typed buffer.

// The SPIR-V pair selector uses one condition for both component choices. We follow its tags
// to the matching A/A and B/B samples, then check the buffer choice against `chooseA` as well.
// The condition checks cover the double negation at `-O0` and its elimination at `-O1`.
// CONDITION0: %[[NOT_A:[^ ]+]] = OpLogicalNot %bool %chooseA{{( |$)}}
// CONDITION0: %[[CHOOSE_A:[^ ]+]] = OpLogicalNot %bool %[[NOT_A]]{{( |$)}}
// CONDITION0: %[[B_TAG:[^ ]+]] = OpSelect %uint %[[CHOOSE_A]] %uint_1 %uint_0{{( |$)}}
// CONDITION0: %[[A_TAG:[^ ]+]] = OpSelect %uint %[[CHOOSE_A]] %uint_3 %uint_2{{( |$)}}
// CONDITION0: OpSelect %uint %[[CHOOSE_A]] %[[A_TAG]] %[[B_TAG]]{{( |$)}}
// CONDITION1: %[[B_TAG:[^ ]+]] = OpSelect %uint %chooseA %uint_1 %uint_0{{( |$)}}
// CONDITION1: %[[A_TAG:[^ ]+]] = OpSelect %uint %chooseA %uint_3 %uint_2{{( |$)}}
// CONDITION1: OpSelect %uint %chooseA %[[A_TAG]] %[[B_TAG]]{{( |$)}}
// SPIRV: %[[NOT_A:[^ ]+]] = OpLogicalNot %bool %chooseA{{( |$)}}
// SPIRV: %[[B_TAG:[^ ]+]] = OpSelect %uint %[[PAIR_CONDITION:[^ ]+]] %uint_1 %uint_0{{( |$)}}
// SPIRV: %[[A_TAG:[^ ]+]] = OpSelect %uint %[[PAIR_CONDITION]] %uint_3 %uint_2{{( |$)}}
// SPIRV: %[[TAG:[^ ]+]] = OpSelect %uint %[[PAIR_CONDITION]] %[[A_TAG]] %[[B_TAG]]{{( |$)}}
// SPIRV: OpSelectionMerge %[[SAMPLE_MERGE:[^ ]+]] None
// SPIRV: OpSwitch %[[TAG]] %[[AA:[^ ]+]] 0 %[[BB:[^ ]+]] 1 %[[BA:[^ ]+]] 2 %[[AB:[^ ]+]]{{( |$)}}
// SPIRV: %[[AA]] = OpLabel
// SPIRV: %[[T_AA:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// SPIRV: %[[S_AA:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// SPIRV: %[[P_AA:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_AA]] %[[S_AA]]{{( |$)}}
// SPIRV: OpImageSampleExplicitLod {{[^ ]+}} %[[P_AA]] {{[^ ]+}} Lod %float_0{{( |$)}}
// SPIRV-NOT: OpLabel
// SPIRV: OpBranch %[[SAMPLE_MERGE]]{{( |$)}}
// SPIRV: %[[BB]] = OpLabel
// SPIRV: %[[T_BB:[^ ]+]] = OpLoad {{[^ ]+}} %textureB{{( |$)}}
// SPIRV: %[[S_BB:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// SPIRV: %[[P_BB:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_BB]] %[[S_BB]]{{( |$)}}
// SPIRV: OpImageSampleExplicitLod {{[^ ]+}} %[[P_BB]] {{[^ ]+}} Lod %float_0{{( |$)}}
// SPIRV-NOT: OpLabel
// SPIRV: OpBranch %[[SAMPLE_MERGE]]{{( |$)}}
// SPIRV: %[[BA]] = OpLabel
// SPIRV: %[[T_BA:[^ ]+]] = OpLoad {{[^ ]+}} %textureB{{( |$)}}
// SPIRV: %[[S_BA:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// SPIRV: %[[P_BA:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_BA]] %[[S_BA]]{{( |$)}}
// SPIRV: OpImageSampleExplicitLod {{[^ ]+}} %[[P_BA]] {{[^ ]+}} Lod %float_0{{( |$)}}
// SPIRV-NOT: OpLabel
// SPIRV: OpBranch %[[SAMPLE_MERGE]]{{( |$)}}
// SPIRV: %[[AB]] = OpLabel
// SPIRV: %[[T_AB:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// SPIRV: %[[S_AB:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// SPIRV: %[[P_AB:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_AB]] %[[S_AB]]{{( |$)}}
// SPIRV: OpImageSampleExplicitLod {{[^ ]+}} %[[P_AB]] {{[^ ]+}} Lod %float_0{{( |$)}}
// SPIRV-NOT: OpLabel
// SPIRV: OpBranch %[[SAMPLE_MERGE]]{{( |$)}}
// SPIRV: %[[SAMPLE_MERGE]] = OpLabel
// SPIRV: OpSelectionMerge %[[BUFFER_MERGE:[^ ]+]] None
// SPIRV: OpBranchConditional %[[NOT_A]] %[[BUFFER_B:[^ ]+]] %[[BUFFER_A:[^ ]+]]{{( |$)}}
// SPIRV: %[[BUFFER_B]] = OpLabel
// SPIRV: %[[B:[^ ]+]] = OpLoad {{[^ ]+}} %bufferB{{( |$)}}
// SPIRV: OpImageFetch {{[^ ]+}} %[[B]] %int_0{{( |$)}}
// SPIRV-NOT: OpLabel
// SPIRV: OpBranch %[[BUFFER_MERGE]]{{( |$)}}
// SPIRV: %[[BUFFER_A]] = OpLabel
// SPIRV: %[[A:[^ ]+]] = OpLoad {{[^ ]+}} %bufferA{{( |$)}}
// SPIRV: OpImageFetch {{[^ ]+}} %[[A]] %int_0{{( |$)}}
// SPIRV-NOT: OpLabel
// SPIRV: OpBranch %[[BUFFER_MERGE]]{{( |$)}}
// SPIRV: %[[BUFFER_MERGE]] = OpLabel
// GLSL_STORAGE: void main()
// GLSL_STORAGE-NOT: texture2D{{[ \t]}}
// GLSL_STORAGE-NOT: sampler{{[ \t]}}
// GLSL_STORAGE-NOT: textureBuffer{{[ \t]}}
// GLSL: bool [[NOT_A:[^ ]+]] = !((gl_FragCoord.x) > 10.0);
// GLSL: if([[NOT_A]])
// GLSL-NEXT: {
// GLSL-NOT: }
// GLSL: if([[NOT_A]])
// GLSL-NEXT: {
// GLSL-NOT: }
// GLSL: textureLod(sampler2D(textureB_0,samplerB_0),
// GLSL-NOT: textureLod
// GLSL-NOT: texelFetch
// GLSL: }
// GLSL-NEXT: else
// GLSL-NEXT: {
// GLSL-NOT: }
// GLSL: textureLod(sampler2D(textureB_0,samplerA_0),
// GLSL-NOT: textureLod
// GLSL-NOT: texelFetch
// GLSL: }
// GLSL-NOT: textureLod
// GLSL: }
// GLSL-NEXT: else
// GLSL-NEXT: {
// GLSL-NOT: }
// GLSL: if([[NOT_A]])
// GLSL-NEXT: {
// GLSL-NOT: }
// GLSL: textureLod(sampler2D(textureA_0,samplerB_0),
// GLSL-NOT: textureLod
// GLSL-NOT: texelFetch
// GLSL: }
// GLSL-NEXT: else
// GLSL-NEXT: {
// GLSL-NOT: }
// GLSL: textureLod(sampler2D(textureA_0,samplerA_0),
// GLSL-NOT: textureLod
// GLSL-NOT: texelFetch
// GLSL: }
// GLSL-NOT: textureLod
// GLSL: }
// GLSL: if([[NOT_A]])
// GLSL-NEXT: {
// GLSL-NOT: }
// GLSL: texelFetch((bufferB_0),
// GLSL-NOT: textureLod
// GLSL-NOT: texelFetch
// GLSL: }
// GLSL-NEXT: else
// GLSL-NEXT: {
// GLSL-NOT: }
// GLSL: texelFetch((bufferA_0),
// GLSL-NOT: textureLod
// GLSL-NOT: texelFetch
// GLSL: }
Texture2D<float4> textureA;
Texture2D<float4> textureB;
SamplerState samplerA;
SamplerState samplerB;
Buffer<float4> bufferA;
Buffer<float4> bufferB;

float4 main(float4 position : SV_Position) : SV_Target
{
    bool chooseA = position.x > 10;
    Texture2D<float4> texture = chooseA ? textureA : textureB;
    SamplerState sampler = chooseA ? samplerA : samplerB;
    Buffer<float4> buffer = chooseA ? bufferA : bufferB;
    return texture.SampleLevel(sampler, position.xy, 0) + buffer.Load(0);
}
