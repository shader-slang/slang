//TEST:SIMPLE(filecheck=SHARED): -target spirv-asm -stage fragment -entry sharedChoices -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SHARED): -target spirv-asm -stage fragment -entry sharedChoices -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL_SHARED): -target glsl -stage fragment -entry sharedChoices -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=LOOP0): -target spirv-asm -stage fragment -entry loop -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=LOOP1): -target spirv-asm -stage fragment -entry loop -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL_LOOP): -target glsl -stage fragment -entry loop -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=INDEPENDENT): -target spirv-asm -stage fragment -entry independent -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=INDEPENDENT): -target spirv-asm -stage fragment -entry independent -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL_INDEPENDENT): -target glsl -stage fragment -entry independent -validate-ir -validate-ir-detailed

// We exercise resource choices that are shared, nested, or carried around a loop.
// The loop swaps two samplers and updates a third, so legalization must preserve several
// mutually dependent resource choices alongside the ordinary loop counter.

Texture2D<float4> textureA;
Texture2D<float4> textureB;
SamplerState samplerA;
SamplerState samplerB;
SamplerState samplerC;
cbuffer Parameters { uint count; };

// Two later choices use `first`, and sampling `first` also keeps a separate consumer live.
// We require legalization to preserve those shared dependencies rather than follow each path
// as an independent tree.
// We follow the nested selector into all three sampler recipes. The separate LOD-1 consumer
// must reuse the same ordinary choice for `first` that appears in the nested selector.
// SHARED: %[[FIRST_FALSE:[^ ]+]] = {{OpLogicalNot|OpFUnordLessThanEqual}} %bool{{( |$)}}
// SHARED: %[[SECOND:[^ ]+]] = OpSelect %bool %[[USE_FIRST:[^ ]+]] %[[FIRST_FALSE]] %false{{( |$)}}
// SHARED: %[[SELECTED_AB:[^ ]+]] = OpSelect %bool %[[OUTER:[^ ]+]] %[[FIRST_FALSE]] %[[SECOND]]{{( |$)}}
// SHARED: %[[AB_TAG:[^ ]+]] = OpSelect %uint %[[SELECTED_AB]] %uint_1 %uint_0{{( |$)}}
// SHARED: %[[NOT_USE_FIRST:[^ ]+]] = OpLogicalNot %bool %[[USE_FIRST]]{{( |$)}}
// SHARED: %[[USE_C:[^ ]+]] = OpSelect %bool %[[OUTER]] %false %[[NOT_USE_FIRST]]{{( |$)}}
// SHARED: %[[TAG:[^ ]+]] = OpSelect %uint %[[USE_C]] %uint_2 %[[AB_TAG]]{{( |$)}}
// SHARED: OpSelectionMerge %[[SELECTED_MERGE:[^ ]+]] None
// SHARED: OpSwitch %[[TAG]] %[[C:[^ ]+]] 0 %[[A:[^ ]+]] 1 %[[B:[^ ]+]]{{( |$)}}
// SHARED: %[[C]] = OpLabel
// SHARED: %[[S_C:[^ ]+]] = OpLoad {{[^ ]+}} %samplerC{{( |$)}}
// SHARED: %[[T_C:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// SHARED: %[[P_C:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_C]] %[[S_C]]{{( |$)}}
// SHARED: OpImageSampleExplicitLod {{[^ ]+}} %[[P_C]] {{[^ ]+}} Lod %float_0{{( |$)}}
// SHARED-NOT: OpLabel
// SHARED: OpBranch %[[SELECTED_MERGE]]{{( |$)}}
// SHARED: %[[A]] = OpLabel
// SHARED: %[[S_A:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// SHARED: %[[T_A:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// SHARED: %[[P_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_A]] %[[S_A]]{{( |$)}}
// SHARED: OpImageSampleExplicitLod {{[^ ]+}} %[[P_A]] {{[^ ]+}} Lod %float_0{{( |$)}}
// SHARED-NOT: OpLabel
// SHARED: OpBranch %[[SELECTED_MERGE]]{{( |$)}}
// SHARED: %[[B]] = OpLabel
// SHARED: %[[S_B:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// SHARED: %[[T_B:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// SHARED: %[[P_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_B]] %[[S_B]]{{( |$)}}
// SHARED: OpImageSampleExplicitLod {{[^ ]+}} %[[P_B]] {{[^ ]+}} Lod %float_0{{( |$)}}
// SHARED-NOT: OpLabel
// SHARED: OpBranch %[[SELECTED_MERGE]]{{( |$)}}
// SHARED: %[[SELECTED_MERGE]] = OpLabel
// SHARED: OpSelectionMerge %[[FIRST_MERGE:[^ ]+]] None
// SHARED: OpBranchConditional %[[FIRST_FALSE]] %[[FIRST_B:[^ ]+]] %[[FIRST_A:[^ ]+]]{{( |$)}}
// SHARED: %[[FIRST_B]] = OpLabel
// SHARED: %[[S_FIRST_B:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// SHARED: %[[T_FIRST_B:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// SHARED: %[[P_FIRST_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_FIRST_B]] %[[S_FIRST_B]]{{( |$)}}
// SHARED: OpImageSampleExplicitLod {{[^ ]+}} %[[P_FIRST_B]] {{[^ ]+}} Lod %float_1{{( |$)}}
// SHARED-NOT: OpLabel
// SHARED: OpBranch %[[FIRST_MERGE]]{{( |$)}}
// SHARED: %[[FIRST_A]] = OpLabel
// SHARED: %[[S_FIRST_A:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// SHARED: %[[T_FIRST_A:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// SHARED: %[[P_FIRST_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_FIRST_A]] %[[S_FIRST_A]]{{( |$)}}
// SHARED: OpImageSampleExplicitLod {{[^ ]+}} %[[P_FIRST_A]] {{[^ ]+}} Lod %float_1{{( |$)}}
// SHARED-NOT: OpLabel
// SHARED: OpBranch %[[FIRST_MERGE]]{{( |$)}}
// SHARED: %[[FIRST_MERGE]] = OpLabel
// GLSL_SHARED: bool [[FIRST_CONDITION:[^ ]+]] = (uv_0.x) > 0.5;
// GLSL_SHARED: uint [[FIRST:[^ ]+]] = [[FIRST_CONDITION]] ? 0U : 1U;
// GLSL_SHARED: switch((uv_0.x) > (uv_0.y) ? [[FIRST]] : ((uv_0.y) > 0.5) ? [[FIRST]] : 2U)
// GLSL_SHARED-NEXT: {
// GLSL_SHARED: case 0U:
// GLSL_SHARED-NEXT: {
// GLSL_SHARED-NOT: }
// GLSL_SHARED: textureLod(sampler2D(textureA_0,samplerA_0), (uv_0), (0.0))
// GLSL_SHARED-NOT: textureLod
// GLSL_SHARED: break;
// GLSL_SHARED-NEXT: }
// GLSL_SHARED: case 1U:
// GLSL_SHARED-NEXT: {
// GLSL_SHARED-NOT: }
// GLSL_SHARED: textureLod(sampler2D(textureA_0,samplerB_0), (uv_0), (0.0))
// GLSL_SHARED-NOT: textureLod
// GLSL_SHARED: break;
// GLSL_SHARED-NEXT: }
// GLSL_SHARED: default:
// GLSL_SHARED-NEXT: {
// GLSL_SHARED-NOT: }
// GLSL_SHARED: textureLod(sampler2D(textureA_0,samplerC_0), (uv_0), (0.0))
// GLSL_SHARED-NOT: textureLod
// GLSL_SHARED: break;
// GLSL_SHARED-NEXT: }
// GLSL_SHARED: }
// GLSL_SHARED: if([[FIRST_CONDITION]])
// GLSL_SHARED-NEXT: {
// GLSL_SHARED-NOT: }
// GLSL_SHARED: textureLod(sampler2D(textureA_0,samplerA_0), (uv_0), (1.0))
// GLSL_SHARED-NOT: textureLod
// GLSL_SHARED-NOT: texelFetch
// GLSL_SHARED: }
// GLSL_SHARED-NEXT: else
// GLSL_SHARED-NEXT: {
// GLSL_SHARED-NOT: }
// GLSL_SHARED: textureLod(sampler2D(textureA_0,samplerB_0), (uv_0), (1.0))
// GLSL_SHARED-NOT: textureLod
// GLSL_SHARED-NOT: texelFetch
// GLSL_SHARED: }
float4 sharedChoices(float2 uv : TEXCOORD) : SV_Target
{
    SamplerState first = select(uv.x > 0.5, samplerA, samplerB);
    SamplerState second = select(uv.y > 0.5, first, samplerC);
    SamplerState selected = select(uv.x > uv.y, first, second);
    return textureA.SampleLevel(selected, uv, 0)
        + textureA.SampleLevel(first, uv, 1);
}

// We swap `first` and `second` on every iteration and conditionally retain or replace `third`.
// The samples after the loop observe all three final choices, including the zero-iteration case.
// We check ordinary selector storage, including the initial values, the conditional update of
// `third`, and the simultaneous swap. The post-loop dispatches must read all three final slots.
// The update of `third` uses the incoming `second`, which is the value assigned to `first`
// in the shader. The ordinary selector slots are swapped afterward, at the loop back edge.
// LOOP0: %first = OpVariable %_ptr_Function_uint Function
// LOOP0: %second = OpVariable %_ptr_Function_uint Function
// LOOP0: %third = OpVariable %_ptr_Function_uint Function
// LOOP0: OpStore %first %uint_0{{( |$)}}
// LOOP0: OpStore %second %uint_1{{( |$)}}
// LOOP0: OpStore %third %uint_2{{( |$)}}
// LOOP0: OpStore %iteration %uint_0{{( |$)}}
// LOOP0: OpBranch %[[HEADER:[^ ]+]]{{( |$)}}
// LOOP0: %[[HEADER]] = OpLabel
// LOOP0: OpLoopMerge %[[AFTER_LOOP:[^ ]+]] %[[BACK_EDGE:[^ ]+]] None
// LOOP0: %[[COUNT_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %Parameters %int_0{{( |$)}}
// LOOP0: %[[COUNT:[^ ]+]] = OpLoad %uint %[[COUNT_PTR]]{{( |$)}}
// LOOP0: %[[ITERATION:[^ ]+]] = OpLoad %uint %iteration{{( |$)}}
// LOOP0: %[[CONTINUE:[^ ]+]] = OpULessThan %bool %[[ITERATION]] %[[COUNT]]{{( |$)}}
// LOOP0: OpBranchConditional %[[CONTINUE]] %[[BODY:[^ ]+]] %[[EXIT:[^ ]+]]{{( |$)}}
// LOOP0: %[[BODY]] = OpLabel
// LOOP0: %[[UPDATE_THIRD:[^ ]+]] = OpFOrdGreaterThan %bool{{( |$)}}
// LOOP0: OpSelectionMerge %[[THIRD_MERGE:[^ ]+]] None
// LOOP0: OpBranchConditional %[[UPDATE_THIRD]] %[[UPDATE:[^ ]+]] %[[THIRD_MERGE]]{{( |$)}}
// LOOP0: %[[UPDATE]] = OpLabel
// LOOP0: %[[USE_SECOND:[^ ]+]] = OpFOrdGreaterThan %bool{{( |$)}}
// LOOP0: %[[INCOMING_SECOND:[^ ]+]] = OpLoad %uint %second{{( |$)}}
// LOOP0: %[[INCOMING_THIRD:[^ ]+]] = OpLoad %uint %third{{( |$)}}
// LOOP0: %[[NEW_THIRD:[^ ]+]] = OpSelect %uint %[[USE_SECOND]] %[[INCOMING_SECOND]] %[[INCOMING_THIRD]]{{( |$)}}
// LOOP0: OpStore %third %[[NEW_THIRD]]{{( |$)}}
// LOOP0-NOT: OpLabel
// LOOP0: OpBranch %[[THIRD_MERGE]]{{( |$)}}
// LOOP0: %[[THIRD_MERGE]] = OpLabel
// LOOP0: %[[OLD_ITERATION:[^ ]+]] = OpLoad %uint %iteration{{( |$)}}
// LOOP0: %[[NEXT_ITERATION:[^ ]+]] = OpIAdd %uint %[[OLD_ITERATION]] %uint_1{{( |$)}}
// LOOP0: OpStore %iteration %[[NEXT_ITERATION]]{{( |$)}}
// LOOP0: %[[OLD_FIRST:[^ ]+]] = OpLoad %uint %first{{( |$)}}
// LOOP0: %[[OLD_SECOND:[^ ]+]] = OpLoad %uint %second{{( |$)}}
// LOOP0: OpStore %first %[[OLD_SECOND]]{{( |$)}}
// LOOP0: OpStore %second %[[OLD_FIRST]]{{( |$)}}
// LOOP0: OpBranch %[[BACK_EDGE]]{{( |$)}}
// LOOP0: %[[BACK_EDGE]] = OpLabel
// LOOP0: OpBranch %[[HEADER]]{{( |$)}}
// LOOP0: %[[EXIT]] = OpLabel
// LOOP0: OpBranch %[[AFTER_LOOP]]{{( |$)}}
// LOOP0: %[[AFTER_LOOP]] = OpLabel
// LOOP0: %[[FINAL_FIRST:[^ ]+]] = OpLoad %uint %first{{( |$)}}
// LOOP0: %[[FIRST_IS_B:[^ ]+]] = OpIEqual %bool %[[FINAL_FIRST]] %uint_1{{( |$)}}
// LOOP0: OpSelectionMerge %[[FIRST_MERGE:[^ ]+]] None
// LOOP0: OpBranchConditional %[[FIRST_IS_B]] %[[FIRST_B:[^ ]+]] %[[FIRST_A:[^ ]+]]{{( |$)}}
// LOOP0: %[[FIRST_B]] = OpLabel
// LOOP0: %[[S_FIRST_B:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// LOOP0: %[[T_FIRST_B:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP0: %[[P_FIRST_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_FIRST_B]] %[[S_FIRST_B]]{{( |$)}}
// LOOP0: OpImageSampleExplicitLod {{[^ ]+}} %[[P_FIRST_B]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP0-NOT: OpLabel
// LOOP0: OpBranch %[[FIRST_MERGE]]{{( |$)}}
// LOOP0: %[[FIRST_A]] = OpLabel
// LOOP0: %[[S_FIRST_A:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// LOOP0: %[[T_FIRST_A:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP0: %[[P_FIRST_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_FIRST_A]] %[[S_FIRST_A]]{{( |$)}}
// LOOP0: OpImageSampleExplicitLod {{[^ ]+}} %[[P_FIRST_A]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP0-NOT: OpLabel
// LOOP0: OpBranch %[[FIRST_MERGE]]{{( |$)}}
// LOOP0: %[[FIRST_MERGE]] = OpLabel
// LOOP0: %[[FINAL_SECOND:[^ ]+]] = OpLoad %uint %second{{( |$)}}
// LOOP0: %[[SECOND_IS_B:[^ ]+]] = OpIEqual %bool %[[FINAL_SECOND]] %uint_1{{( |$)}}
// LOOP0: OpSelectionMerge %[[SECOND_MERGE:[^ ]+]] None
// LOOP0: OpBranchConditional %[[SECOND_IS_B]] %[[SECOND_B:[^ ]+]] %[[SECOND_A:[^ ]+]]{{( |$)}}
// LOOP0: %[[SECOND_B]] = OpLabel
// LOOP0: %[[S_SECOND_B:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// LOOP0: %[[T_SECOND_B:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP0: %[[P_SECOND_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_SECOND_B]] %[[S_SECOND_B]]{{( |$)}}
// LOOP0: OpImageSampleExplicitLod {{[^ ]+}} %[[P_SECOND_B]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP0-NOT: OpLabel
// LOOP0: OpBranch %[[SECOND_MERGE]]{{( |$)}}
// LOOP0: %[[SECOND_A]] = OpLabel
// LOOP0: %[[S_SECOND_A:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// LOOP0: %[[T_SECOND_A:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP0: %[[P_SECOND_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_SECOND_A]] %[[S_SECOND_A]]{{( |$)}}
// LOOP0: OpImageSampleExplicitLod {{[^ ]+}} %[[P_SECOND_A]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP0-NOT: OpLabel
// LOOP0: OpBranch %[[SECOND_MERGE]]{{( |$)}}
// LOOP0: %[[SECOND_MERGE]] = OpLabel
// LOOP0: %[[FINAL_THIRD:[^ ]+]] = OpLoad %uint %third{{( |$)}}
// LOOP0: %[[THIRD_IS_A:[^ ]+]] = OpIEqual %bool %[[FINAL_THIRD]] %uint_0{{( |$)}}
// LOOP0: %[[THIRD_A_TAG:[^ ]+]] = OpSelect %uint %[[THIRD_IS_A]] %uint_0 %uint_2{{( |$)}}
// LOOP0: %[[FINAL_THIRD_B:[^ ]+]] = OpLoad %uint %third{{( |$)}}
// LOOP0: %[[THIRD_IS_B:[^ ]+]] = OpIEqual %bool %[[FINAL_THIRD_B]] %uint_1{{( |$)}}
// LOOP0: %[[THIRD_TAG:[^ ]+]] = OpSelect %uint %[[THIRD_IS_B]] %uint_1 %[[THIRD_A_TAG]]{{( |$)}}
// LOOP0: OpSelectionMerge %[[THIRD_RESULT_MERGE:[^ ]+]] None
// LOOP0: OpSwitch %[[THIRD_TAG]] %[[THIRD_B:[^ ]+]] 2 %[[THIRD_C:[^ ]+]] 0 %[[THIRD_A:[^ ]+]]{{( |$)}}
// LOOP0: %[[THIRD_B]] = OpLabel
// LOOP0: %[[S_THIRD_B:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// LOOP0: %[[T_THIRD_B:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP0: %[[P_THIRD_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_THIRD_B]] %[[S_THIRD_B]]{{( |$)}}
// LOOP0: OpImageSampleExplicitLod {{[^ ]+}} %[[P_THIRD_B]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP0-NOT: OpLabel
// LOOP0: OpBranch %[[THIRD_RESULT_MERGE]]{{( |$)}}
// LOOP0: %[[THIRD_C]] = OpLabel
// LOOP0: %[[S_THIRD_C:[^ ]+]] = OpLoad {{[^ ]+}} %samplerC{{( |$)}}
// LOOP0: %[[T_THIRD_C:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP0: %[[P_THIRD_C:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_THIRD_C]] %[[S_THIRD_C]]{{( |$)}}
// LOOP0: OpImageSampleExplicitLod {{[^ ]+}} %[[P_THIRD_C]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP0-NOT: OpLabel
// LOOP0: OpBranch %[[THIRD_RESULT_MERGE]]{{( |$)}}
// LOOP0: %[[THIRD_A]] = OpLabel
// LOOP0: %[[S_THIRD_A:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// LOOP0: %[[T_THIRD_A:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP0: %[[P_THIRD_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_THIRD_A]] %[[S_THIRD_A]]{{( |$)}}
// LOOP0: OpImageSampleExplicitLod {{[^ ]+}} %[[P_THIRD_A]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP0-NOT: OpLabel
// LOOP0: OpBranch %[[THIRD_RESULT_MERGE]]{{( |$)}}
// LOOP0: %[[THIRD_RESULT_MERGE]] = OpLabel

// The optimized checks account for the loop exit preceding the body and the sampler arms
// appearing in a different order. They require the same selector updates and merge edges.
// LOOP1: %first = OpVariable %_ptr_Function_uint Function
// LOOP1: %second = OpVariable %_ptr_Function_uint Function
// LOOP1: %third = OpVariable %_ptr_Function_uint Function
// LOOP1: OpStore %first %uint_0{{( |$)}}
// LOOP1: OpStore %second %uint_1{{( |$)}}
// LOOP1: OpStore %third %uint_2{{( |$)}}
// LOOP1: OpStore %iteration %uint_0{{( |$)}}
// LOOP1: OpBranch %[[HEADER:[^ ]+]]{{( |$)}}
// LOOP1: %[[HEADER]] = OpLabel
// LOOP1: OpLoopMerge %[[AFTER_LOOP:[^ ]+]] %[[BACK_EDGE:[^ ]+]] None
// LOOP1: %[[COUNT_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %Parameters %int_0{{( |$)}}
// LOOP1: %[[COUNT:[^ ]+]] = OpLoad %uint %[[COUNT_PTR]]{{( |$)}}
// LOOP1: %[[ITERATION:[^ ]+]] = OpLoad %uint %iteration{{( |$)}}
// LOOP1: %[[CONTINUE:[^ ]+]] = OpULessThan %bool %[[ITERATION]] %[[COUNT]]{{( |$)}}
// LOOP1: OpBranchConditional %[[CONTINUE]] %[[BODY:[^ ]+]] %[[EXIT:[^ ]+]]{{( |$)}}
// LOOP1: %[[EXIT]] = OpLabel
// LOOP1: OpBranch %[[AFTER_LOOP]]{{( |$)}}
// LOOP1: %[[BODY]] = OpLabel
// LOOP1: %[[UPDATE_THIRD:[^ ]+]] = OpFOrdGreaterThan %bool{{( |$)}}
// LOOP1: OpSelectionMerge %[[THIRD_MERGE:[^ ]+]] None
// LOOP1: OpBranchConditional %[[UPDATE_THIRD]] %[[UPDATE:[^ ]+]] %[[THIRD_MERGE]]{{( |$)}}
// LOOP1: %[[UPDATE]] = OpLabel
// LOOP1: %[[USE_SECOND:[^ ]+]] = OpFOrdGreaterThan %bool{{( |$)}}
// LOOP1: %[[INCOMING_SECOND:[^ ]+]] = OpLoad %uint %second{{( |$)}}
// LOOP1: %[[INCOMING_THIRD:[^ ]+]] = OpLoad %uint %third{{( |$)}}
// LOOP1: %[[NEW_THIRD:[^ ]+]] = OpSelect %uint %[[USE_SECOND]] %[[INCOMING_SECOND]] %[[INCOMING_THIRD]]{{( |$)}}
// LOOP1: OpStore %third %[[NEW_THIRD]]{{( |$)}}
// LOOP1-NOT: OpLabel
// LOOP1: OpBranch %[[THIRD_MERGE]]{{( |$)}}
// LOOP1: %[[THIRD_MERGE]] = OpLabel
// LOOP1: %[[OLD_ITERATION:[^ ]+]] = OpLoad %uint %iteration{{( |$)}}
// LOOP1: %[[NEXT_ITERATION:[^ ]+]] = OpIAdd %uint %[[OLD_ITERATION]] %uint_1{{( |$)}}
// LOOP1: OpStore %iteration %[[NEXT_ITERATION]]{{( |$)}}
// LOOP1: %[[OLD_FIRST:[^ ]+]] = OpLoad %uint %first{{( |$)}}
// LOOP1: %[[OLD_SECOND:[^ ]+]] = OpLoad %uint %second{{( |$)}}
// LOOP1: OpStore %first %[[OLD_SECOND]]{{( |$)}}
// LOOP1: OpStore %second %[[OLD_FIRST]]{{( |$)}}
// LOOP1: OpBranch %[[BACK_EDGE]]{{( |$)}}
// LOOP1: %[[BACK_EDGE]] = OpLabel
// LOOP1: OpBranch %[[HEADER]]{{( |$)}}
// LOOP1: %[[AFTER_LOOP]] = OpLabel
// LOOP1: %[[FINAL_FIRST:[^ ]+]] = OpLoad %uint %first{{( |$)}}
// LOOP1: %[[FIRST_IS_B:[^ ]+]] = OpIEqual %bool %[[FINAL_FIRST]] %uint_1{{( |$)}}
// LOOP1: OpSelectionMerge %[[FIRST_MERGE:[^ ]+]] None
// LOOP1: OpBranchConditional %[[FIRST_IS_B]] %[[FIRST_B:[^ ]+]] %[[FIRST_A:[^ ]+]]{{( |$)}}
// LOOP1: %[[FIRST_A]] = OpLabel
// LOOP1: %[[S_FIRST_A:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// LOOP1: %[[T_FIRST_A:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP1: %[[P_FIRST_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_FIRST_A]] %[[S_FIRST_A]]{{( |$)}}
// LOOP1: OpImageSampleExplicitLod {{[^ ]+}} %[[P_FIRST_A]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP1-NOT: OpLabel
// LOOP1: OpBranch %[[FIRST_MERGE]]{{( |$)}}
// LOOP1: %[[FIRST_B]] = OpLabel
// LOOP1: %[[S_FIRST_B:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// LOOP1: %[[T_FIRST_B:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP1: %[[P_FIRST_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_FIRST_B]] %[[S_FIRST_B]]{{( |$)}}
// LOOP1: OpImageSampleExplicitLod {{[^ ]+}} %[[P_FIRST_B]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP1-NOT: OpLabel
// LOOP1: OpBranch %[[FIRST_MERGE]]{{( |$)}}
// LOOP1: %[[FIRST_MERGE]] = OpLabel
// LOOP1: %[[FINAL_SECOND:[^ ]+]] = OpLoad %uint %second{{( |$)}}
// LOOP1: %[[SECOND_IS_B:[^ ]+]] = OpIEqual %bool %[[FINAL_SECOND]] %uint_1{{( |$)}}
// LOOP1: OpSelectionMerge %[[SECOND_MERGE:[^ ]+]] None
// LOOP1: OpBranchConditional %[[SECOND_IS_B]] %[[SECOND_B:[^ ]+]] %[[SECOND_A:[^ ]+]]{{( |$)}}
// LOOP1: %[[SECOND_A]] = OpLabel
// LOOP1: %[[S_SECOND_A:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// LOOP1: %[[T_SECOND_A:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP1: %[[P_SECOND_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_SECOND_A]] %[[S_SECOND_A]]{{( |$)}}
// LOOP1: OpImageSampleExplicitLod {{[^ ]+}} %[[P_SECOND_A]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP1-NOT: OpLabel
// LOOP1: OpBranch %[[SECOND_MERGE]]{{( |$)}}
// LOOP1: %[[SECOND_B]] = OpLabel
// LOOP1: %[[S_SECOND_B:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// LOOP1: %[[T_SECOND_B:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP1: %[[P_SECOND_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_SECOND_B]] %[[S_SECOND_B]]{{( |$)}}
// LOOP1: OpImageSampleExplicitLod {{[^ ]+}} %[[P_SECOND_B]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP1-NOT: OpLabel
// LOOP1: OpBranch %[[SECOND_MERGE]]{{( |$)}}
// LOOP1: %[[SECOND_MERGE]] = OpLabel
// LOOP1: %[[FINAL_THIRD:[^ ]+]] = OpLoad %uint %third{{( |$)}}
// LOOP1: %[[THIRD_IS_A:[^ ]+]] = OpIEqual %bool %[[FINAL_THIRD]] %uint_0{{( |$)}}
// LOOP1: %[[THIRD_A_TAG:[^ ]+]] = OpSelect %uint %[[THIRD_IS_A]] %uint_0 %uint_2{{( |$)}}
// LOOP1: %[[THIRD_IS_B:[^ ]+]] = OpIEqual %bool %[[FINAL_THIRD]] %uint_1{{( |$)}}
// LOOP1: %[[THIRD_TAG:[^ ]+]] = OpSelect %uint %[[THIRD_IS_B]] %uint_1 %[[THIRD_A_TAG]]{{( |$)}}
// LOOP1: OpSelectionMerge %[[THIRD_RESULT_MERGE:[^ ]+]] None
// LOOP1: OpSwitch %[[THIRD_TAG]] %[[THIRD_B:[^ ]+]] 2 %[[THIRD_C:[^ ]+]] 0 %[[THIRD_A:[^ ]+]]{{( |$)}}
// LOOP1: %[[THIRD_A]] = OpLabel
// LOOP1: %[[S_THIRD_A:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// LOOP1: %[[T_THIRD_A:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP1: %[[P_THIRD_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_THIRD_A]] %[[S_THIRD_A]]{{( |$)}}
// LOOP1: OpImageSampleExplicitLod {{[^ ]+}} %[[P_THIRD_A]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP1-NOT: OpLabel
// LOOP1: OpBranch %[[THIRD_RESULT_MERGE]]{{( |$)}}
// LOOP1: %[[THIRD_C]] = OpLabel
// LOOP1: %[[S_THIRD_C:[^ ]+]] = OpLoad {{[^ ]+}} %samplerC{{( |$)}}
// LOOP1: %[[T_THIRD_C:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP1: %[[P_THIRD_C:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_THIRD_C]] %[[S_THIRD_C]]{{( |$)}}
// LOOP1: OpImageSampleExplicitLod {{[^ ]+}} %[[P_THIRD_C]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP1-NOT: OpLabel
// LOOP1: OpBranch %[[THIRD_RESULT_MERGE]]{{( |$)}}
// LOOP1: %[[THIRD_B]] = OpLabel
// LOOP1: %[[S_THIRD_B:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// LOOP1: %[[T_THIRD_B:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// LOOP1: %[[P_THIRD_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_THIRD_B]] %[[S_THIRD_B]]{{( |$)}}
// LOOP1: OpImageSampleExplicitLod {{[^ ]+}} %[[P_THIRD_B]] {{[^ ]+}} Lod %float_0{{( |$)}}
// LOOP1-NOT: OpLabel
// LOOP1: OpBranch %[[THIRD_RESULT_MERGE]]{{( |$)}}
// LOOP1: %[[THIRD_RESULT_MERGE]] = OpLabel
// GLSL_LOOP: uint [[FIRST:[^ ]+]] = 0U;
// GLSL_LOOP: uint [[SECOND:[^ ]+]] = 1U;
// GLSL_LOOP: uint [[THIRD:[^ ]+]] = 2U;
// GLSL_LOOP: uint [[ITERATION:[^ ]+]] = 0U;
// GLSL_LOOP: for(;;)
// GLSL_LOOP-NEXT: {
// GLSL_LOOP: if([[ITERATION]] < (Parameters_0.count_0))
// GLSL_LOOP-NEXT: {
// GLSL_LOOP-NEXT: }
// GLSL_LOOP-NEXT: else
// GLSL_LOOP-NEXT: {
// GLSL_LOOP: break;
// GLSL_LOOP: }
// GLSL_LOOP: if((uv_0.x) > float([[ITERATION]]))
// GLSL_LOOP-NEXT: {
// GLSL_LOOP-NOT: }
// GLSL_LOOP: [[THIRD]] = (uv_0.y) > 0.5 ? [[SECOND]] : [[THIRD]];
// GLSL_LOOP: }
// GLSL_LOOP: [[ITERATION]] = [[ITERATION]] + 1U;
// GLSL_LOOP: uint [[OLD_FIRST:[^ ]+]] = [[FIRST]];
// GLSL_LOOP: [[FIRST]] = [[SECOND]];
// GLSL_LOOP: [[SECOND]] = [[OLD_FIRST]];
// GLSL_LOOP: }
// GLSL_LOOP: if([[FIRST]] == 0U)
// GLSL_LOOP-NEXT: {
// GLSL_LOOP-NOT: }
// GLSL_LOOP: textureLod(sampler2D(textureA_0,samplerA_0),
// GLSL_LOOP-NOT: textureLod
// GLSL_LOOP-NOT: texelFetch
// GLSL_LOOP: }
// GLSL_LOOP-NEXT: else
// GLSL_LOOP-NEXT: {
// GLSL_LOOP-NOT: }
// GLSL_LOOP: textureLod(sampler2D(textureA_0,samplerB_0),
// GLSL_LOOP-NOT: textureLod
// GLSL_LOOP-NOT: texelFetch
// GLSL_LOOP: }
// GLSL_LOOP: if([[SECOND]] == 0U)
// GLSL_LOOP-NEXT: {
// GLSL_LOOP-NOT: }
// GLSL_LOOP: textureLod(sampler2D(textureA_0,samplerA_0),
// GLSL_LOOP-NOT: textureLod
// GLSL_LOOP-NOT: texelFetch
// GLSL_LOOP: }
// GLSL_LOOP-NEXT: else
// GLSL_LOOP-NEXT: {
// GLSL_LOOP-NOT: }
// GLSL_LOOP: textureLod(sampler2D(textureA_0,samplerB_0),
// GLSL_LOOP-NOT: textureLod
// GLSL_LOOP-NOT: texelFetch
// GLSL_LOOP: }
// GLSL_LOOP: switch([[THIRD]])
// GLSL_LOOP-NEXT: {
// GLSL_LOOP: case 2U:
// GLSL_LOOP-NEXT: {
// GLSL_LOOP-NOT: }
// GLSL_LOOP: textureLod(sampler2D(textureA_0,samplerC_0),
// GLSL_LOOP-NOT: textureLod
// GLSL_LOOP: break;
// GLSL_LOOP-NEXT: }
// GLSL_LOOP: case 0U:
// GLSL_LOOP-NEXT: {
// GLSL_LOOP-NOT: }
// GLSL_LOOP: textureLod(sampler2D(textureA_0,samplerA_0),
// GLSL_LOOP-NOT: textureLod
// GLSL_LOOP: break;
// GLSL_LOOP-NEXT: }
// GLSL_LOOP: default:
// GLSL_LOOP-NEXT: {
// GLSL_LOOP-NOT: }
// GLSL_LOOP: textureLod(sampler2D(textureA_0,samplerB_0),
// GLSL_LOOP-NOT: textureLod
// GLSL_LOOP: break;
// GLSL_LOOP-NEXT: }
// GLSL_LOOP: }
float4 loop(float2 uv : TEXCOORD) : SV_Target
{
    SamplerState first = samplerA;
    SamplerState second = samplerB;
    SamplerState third = samplerC;
    for (uint iteration = 0; iteration < count; iteration++)
    {
        SamplerState previousFirst = first;
        first = second;
        second = previousFirst;
        if (uv.x > float(iteration))
            third = select(uv.y > 0.5, first, third);
    }
    return textureA.SampleLevel(first, uv, 0)
        + 2 * textureA.SampleLevel(second, uv, 0)
        + 4 * textureA.SampleLevel(third, uv, 0);
}

// The texture and sampler depend on different conditions. A sample must preserve all four
// combinations rather than assume that separately selected operands use the same branch.
// The selector uses the sampler condition inside each texture alternative. We follow all four
// switch cases to the actual texture/sampler pair consumed by the sample.
// INDEPENDENT: %[[TEXTURE_CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool{{( |$)}}
// INDEPENDENT: %[[SAMPLER_CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool{{( |$)}}
// INDEPENDENT: %[[NOT_SAMPLER:[^ ]+]] = OpLogicalNot %bool %[[SAMPLER_CONDITION]]{{( |$)}}
// INDEPENDENT: %[[A_TAG:[^ ]+]] = OpSelect %uint %[[NOT_SAMPLER]] %uint_1 %uint_0{{( |$)}}
// INDEPENDENT: %[[B_TAG:[^ ]+]] = OpSelect %uint %[[NOT_SAMPLER]] %uint_3 %uint_2{{( |$)}}
// INDEPENDENT: %[[NOT_TEXTURE:[^ ]+]] = OpLogicalNot %bool %[[TEXTURE_CONDITION]]{{( |$)}}
// INDEPENDENT: %[[TAG:[^ ]+]] = OpSelect %uint %[[NOT_TEXTURE]] %[[B_TAG]] %[[A_TAG]]{{( |$)}}
// INDEPENDENT: OpSelectionMerge %[[MERGE:[^ ]+]] None
// INDEPENDENT: OpSwitch %[[TAG]] %[[BB:[^ ]+]] 0 %[[AA:[^ ]+]] 1 %[[AB:[^ ]+]] 2 %[[BA:[^ ]+]]{{( |$)}}
// INDEPENDENT: %[[BB]] = OpLabel
// INDEPENDENT: %[[T_BB:[^ ]+]] = OpLoad {{[^ ]+}} %textureB{{( |$)}}
// INDEPENDENT: %[[S_BB:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// INDEPENDENT: %[[P_BB:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_BB]] %[[S_BB]]{{( |$)}}
// INDEPENDENT: OpImageSampleExplicitLod {{[^ ]+}} %[[P_BB]] {{[^ ]+}} Lod %float_0{{( |$)}}
// INDEPENDENT-NOT: OpLabel
// INDEPENDENT: OpBranch %[[MERGE]]{{( |$)}}
// INDEPENDENT: %[[AA]] = OpLabel
// INDEPENDENT: %[[T_AA:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// INDEPENDENT: %[[S_AA:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// INDEPENDENT: %[[P_AA:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_AA]] %[[S_AA]]{{( |$)}}
// INDEPENDENT: OpImageSampleExplicitLod {{[^ ]+}} %[[P_AA]] {{[^ ]+}} Lod %float_0{{( |$)}}
// INDEPENDENT-NOT: OpLabel
// INDEPENDENT: OpBranch %[[MERGE]]{{( |$)}}
// INDEPENDENT: %[[AB]] = OpLabel
// INDEPENDENT: %[[T_AB:[^ ]+]] = OpLoad {{[^ ]+}} %textureA{{( |$)}}
// INDEPENDENT: %[[S_AB:[^ ]+]] = OpLoad {{[^ ]+}} %samplerB{{( |$)}}
// INDEPENDENT: %[[P_AB:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_AB]] %[[S_AB]]{{( |$)}}
// INDEPENDENT: OpImageSampleExplicitLod {{[^ ]+}} %[[P_AB]] {{[^ ]+}} Lod %float_0{{( |$)}}
// INDEPENDENT-NOT: OpLabel
// INDEPENDENT: OpBranch %[[MERGE]]{{( |$)}}
// INDEPENDENT: %[[BA]] = OpLabel
// INDEPENDENT: %[[T_BA:[^ ]+]] = OpLoad {{[^ ]+}} %textureB{{( |$)}}
// INDEPENDENT: %[[S_BA:[^ ]+]] = OpLoad {{[^ ]+}} %samplerA{{( |$)}}
// INDEPENDENT: %[[P_BA:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[T_BA]] %[[S_BA]]{{( |$)}}
// INDEPENDENT: OpImageSampleExplicitLod {{[^ ]+}} %[[P_BA]] {{[^ ]+}} Lod %float_0{{( |$)}}
// INDEPENDENT-NOT: OpLabel
// INDEPENDENT: OpBranch %[[MERGE]]{{( |$)}}
// INDEPENDENT: %[[MERGE]] = OpLabel
// GLSL_INDEPENDENT: bool [[SAMPLER:[^ ]+]] = (uv_0.y) > 0.5;
// GLSL_INDEPENDENT: if((uv_0.x) > 0.5)
// GLSL_INDEPENDENT-NEXT: {
// GLSL_INDEPENDENT-NOT: }
// GLSL_INDEPENDENT: if([[SAMPLER]])
// GLSL_INDEPENDENT-NEXT: {
// GLSL_INDEPENDENT-NOT: }
// GLSL_INDEPENDENT: textureLod(sampler2D(textureA_0,samplerA_0),
// GLSL_INDEPENDENT-NOT: textureLod
// GLSL_INDEPENDENT-NOT: texelFetch
// GLSL_INDEPENDENT: }
// GLSL_INDEPENDENT-NEXT: else
// GLSL_INDEPENDENT-NEXT: {
// GLSL_INDEPENDENT-NOT: }
// GLSL_INDEPENDENT: textureLod(sampler2D(textureA_0,samplerB_0),
// GLSL_INDEPENDENT-NOT: textureLod
// GLSL_INDEPENDENT-NOT: texelFetch
// GLSL_INDEPENDENT: }
// GLSL_INDEPENDENT-NOT: textureLod
// GLSL_INDEPENDENT: }
// GLSL_INDEPENDENT-NEXT: else
// GLSL_INDEPENDENT-NEXT: {
// GLSL_INDEPENDENT-NOT: }
// GLSL_INDEPENDENT: if([[SAMPLER]])
// GLSL_INDEPENDENT-NEXT: {
// GLSL_INDEPENDENT-NOT: }
// GLSL_INDEPENDENT: textureLod(sampler2D(textureB_0,samplerA_0),
// GLSL_INDEPENDENT-NOT: textureLod
// GLSL_INDEPENDENT-NOT: texelFetch
// GLSL_INDEPENDENT: }
// GLSL_INDEPENDENT-NEXT: else
// GLSL_INDEPENDENT-NEXT: {
// GLSL_INDEPENDENT-NOT: }
// GLSL_INDEPENDENT: textureLod(sampler2D(textureB_0,samplerB_0),
// GLSL_INDEPENDENT-NOT: textureLod
// GLSL_INDEPENDENT-NOT: texelFetch
// GLSL_INDEPENDENT: }
// GLSL_INDEPENDENT-NOT: textureLod
// GLSL_INDEPENDENT: }
float4 independent(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(uv.x > 0.5, textureA, textureB);
    SamplerState sampler = select(uv.y > 0.5, samplerA, samplerB);
    return texture.SampleLevel(sampler, uv, 0);
}
