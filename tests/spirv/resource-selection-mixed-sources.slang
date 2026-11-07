//TEST:SIMPLE(filecheck=BRANCH): -target spirv-asm -stage fragment -entry branchIndex -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=BRANCH): -target spirv-asm -stage fragment -entry branchIndex -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL_BRANCH): -target glsl -stage fragment -entry branchIndex -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=ARRAYS): -target spirv-asm -stage fragment -entry differentArrays -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=ARRAYS): -target spirv-asm -stage fragment -entry differentArrays -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL_ARRAYS): -target glsl -stage fragment -entry differentArrays -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL_NESTED): -target glsl -stage fragment -entry nestedArrays -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SELECTED_ARRAY): -target spirv-asm -stage fragment -entry selectedArray -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SELECTED_ARRAY): -target spirv-asm -stage fragment -entry selectedArray -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL_ARRAY): -target glsl -stage fragment -entry selectedArray -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL_ROW): -target glsl -stage fragment -entry selectedRow -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=HEAP): -target spirv-asm -stage fragment -entry heapOrArray -capability spvDescriptorHeapEXT -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=HEAP): -target spirv-asm -stage fragment -entry heapOrArray -capability spvDescriptorHeapEXT -validate-ir -validate-ir-detailed -O1

// We select resources reached through different declarations and through different numbers
// of array subscripts. Descriptor indices must remain ordinary SSA values even when an index
// is computed only on one incoming branch or the selected value is itself an array.
// The direct SPIR-V emitter retains nested descriptor array types, which Vulkan rejects even
// without a resource selection. We exercise multidimensional arrays and selected rows through
// GLSL output and use one-dimensional descriptor arrays for the SPIR-V validation checks.

Texture2D<float4> singleTexture;
Texture2D<float4> firstTextures[4];
Texture2D<float4> secondTextures[4];
Texture2D<float4> textureRows[2][4];
Texture2D<float4> otherRows[2][4];
SamplerState samplers[4];
cbuffer Parameters { uint index; uint otherIndex; };

// The selected descriptor index is computed inside the branch, while the sample coordinates
// are computed after the merge. We require both values to remain available at the sample.
// We follow the branch-only index through its ordinary merge slot into the array access.
// Both samples must use the coordinate computed after that merge and rejoin one result block.
// BRANCH: %[[CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool{{( |$)}}
// BRANCH: OpSelectionMerge %[[INDEX_MERGE:[^ ]+]] None
// BRANCH: OpBranchConditional %[[CONDITION]] %[[COMPUTE:[^ ]+]] %[[DEFAULT:[^ ]+]]{{( |$)}}
// BRANCH: %[[COMPUTE]] = OpLabel
// BRANCH: %[[INDEX:[^ ]+]] = OpUMod %uint {{[^ ]+}} %uint_4{{( |$)}}
// BRANCH: OpStore %[[INDEX_SLOT:[^ ]+]] %[[INDEX]]{{( |$)}}
// BRANCH-NOT: OpLabel
// BRANCH: OpBranch %[[INDEX_MERGE]]{{( |$)}}
// BRANCH: %[[DEFAULT]] = OpLabel
// BRANCH: OpStore %[[INDEX_SLOT]] %uint_0{{( |$)}}
// BRANCH-NOT: OpLabel
// BRANCH: OpBranch %[[INDEX_MERGE]]{{( |$)}}
// BRANCH: %[[INDEX_MERGE]] = OpLabel
// BRANCH: %[[USE_SINGLE:[^ ]+]] = OpLogicalNot %bool %[[CONDITION]]{{( |$)}}
// BRANCH: %coordinate = OpFAdd %v2float{{( |$)}}
// BRANCH: OpSelectionMerge %[[RESULT_MERGE:[^ ]+]] None
// BRANCH: OpBranchConditional %[[USE_SINGLE]] %[[SINGLE:[^ ]+]] %[[ARRAY:[^ ]+]]{{( |$)}}
// BRANCH: %[[SINGLE]] = OpLabel
// BRANCH: %[[SINGLE_SAMPLER_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %uint_0{{( |$)}}
// BRANCH: %[[SINGLE_IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %singleTexture{{( |$)}}
// BRANCH: %[[SINGLE_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[SINGLE_SAMPLER_PTR]]{{( |$)}}
// BRANCH: %[[SINGLE_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[SINGLE_IMAGE]] %[[SINGLE_SAMPLER]]{{( |$)}}
// BRANCH: OpImageSampleExplicitLod {{[^ ]+}} %[[SINGLE_PAIR]] %coordinate Lod %float_0{{( |$)}}
// BRANCH-NOT: OpLabel
// BRANCH: OpBranch %[[RESULT_MERGE]]{{( |$)}}
// BRANCH: %[[ARRAY]] = OpLabel
// BRANCH: %[[FINAL_INDEX:[^ ]+]] = OpLoad %uint %[[INDEX_SLOT]]{{( |$)}}
// BRANCH: %[[ARRAY_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %firstTextures %[[FINAL_INDEX]]{{( |$)}}
// BRANCH: %[[ARRAY_SAMPLER_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %uint_0{{( |$)}}
// BRANCH: %[[ARRAY_IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %[[ARRAY_PTR]]{{( |$)}}
// BRANCH: %[[ARRAY_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[ARRAY_SAMPLER_PTR]]{{( |$)}}
// BRANCH: %[[ARRAY_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[ARRAY_IMAGE]] %[[ARRAY_SAMPLER]]{{( |$)}}
// BRANCH: OpImageSampleExplicitLod {{[^ ]+}} %[[ARRAY_PAIR]] %coordinate Lod %float_0{{( |$)}}
// BRANCH-NOT: OpLabel
// BRANCH: OpBranch %[[RESULT_MERGE]]{{( |$)}}
// BRANCH: %[[RESULT_MERGE]] = OpLabel
// GLSL_BRANCH: bool [[CONDITION:[^ ]+]] = (uv_0.x) > 0.5;
// GLSL_BRANCH: if([[CONDITION]])
// GLSL_BRANCH-NEXT: {
// GLSL_BRANCH-NOT: }
// GLSL_BRANCH: [[INDEX:[^ ]+]] = (Parameters_0.index_0 + uint(uv_0.y)) % 4U;
// GLSL_BRANCH: }
// GLSL_BRANCH-NEXT: else
// GLSL_BRANCH-NEXT: {
// GLSL_BRANCH-NOT: }
// GLSL_BRANCH: [[INDEX]] = 0U;
// GLSL_BRANCH: }
// GLSL_BRANCH: vec2 [[COORDINATE:[^ ]+]] = uv_0 * 0.5 + 0.25;
// GLSL_BRANCH: if(![[CONDITION]])
// GLSL_BRANCH-NEXT: {
// GLSL_BRANCH-NOT: }
// GLSL_BRANCH: textureLod(sampler2D(singleTexture_0,samplers_0[0U]), ([[COORDINATE]]),
// GLSL_BRANCH-NOT: textureLod
// GLSL_BRANCH-NOT: texelFetch
// GLSL_BRANCH: }
// GLSL_BRANCH-NEXT: else
// GLSL_BRANCH-NEXT: {
// GLSL_BRANCH-NOT: }
// GLSL_BRANCH: textureLod(sampler2D(firstTextures_0[nonuniformEXT([[INDEX]])],samplers_0[0U]), ([[COORDINATE]]),
// GLSL_BRANCH-NOT: textureLod
// GLSL_BRANCH-NOT: texelFetch
// GLSL_BRANCH: }
float4 branchIndex(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> selected = singleTexture;
    if (uv.x > 0.5)
    {
        uint branchIndex = (index + uint(uv.y)) % 4;
        selected = firstTextures[branchIndex];
    }
    float2 coordinate = uv * 0.5 + 0.25;
    return selected.SampleLevel(samplers[0], coordinate, 0);
}

// Each array keeps the payload computed from its own cbuffer field, and only the chosen
// branch loads that array element and samples it before reaching the shared result merge.
// ARRAYS: %selected = OpFOrdGreaterThan %bool{{( |$)}}
// ARRAYS: %[[FIRST_FIELD:[^ ]+]] = OpAccessChain {{[^ ]+}} %Parameters %int_0{{( |$)}}
// ARRAYS: %[[FIRST_INDEX:[^ ]+]] = OpLoad %uint %[[FIRST_FIELD]]{{( |$)}}
// ARRAYS: %[[FIRST_MOD:[^ ]+]] = OpUMod %uint %[[FIRST_INDEX]] %uint_4{{( |$)}}
// ARRAYS: %[[SECOND_FIELD:[^ ]+]] = OpAccessChain {{[^ ]+}} %Parameters %int_1{{( |$)}}
// ARRAYS: %[[SECOND_INDEX:[^ ]+]] = OpLoad %uint %[[SECOND_FIELD]]{{( |$)}}
// ARRAYS: %[[SECOND_MOD:[^ ]+]] = OpUMod %uint %[[SECOND_INDEX]] %uint_4{{( |$)}}
// ARRAYS: %[[FIRST_PAYLOAD:[^ ]+]] = OpSelect %uint %selected %[[FIRST_MOD]] %uint_0{{( |$)}}
// ARRAYS: %[[SECOND_PAYLOAD:[^ ]+]] = OpSelect %uint %selected %uint_0 %[[SECOND_MOD]]{{( |$)}}
// ARRAYS: OpSelectionMerge %[[MERGE:[^ ]+]] None
// ARRAYS: OpBranchConditional %selected %[[FIRST:[^ ]+]] %[[SECOND:[^ ]+]]{{( |$)}}
// ARRAYS: %[[FIRST]] = OpLabel
// ARRAYS: %[[IMAGE_PTR_A:[^ ]+]] = OpAccessChain {{[^ ]+}} %firstTextures %[[FIRST_PAYLOAD]]{{( |$)}}
// ARRAYS: %[[SAMPLER_PTR_A:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %uint_0{{( |$)}}
// ARRAYS: %[[IMAGE_A:[^ ]+]] = OpLoad {{[^ ]+}} %[[IMAGE_PTR_A]]{{( |$)}}
// ARRAYS: %[[SAMPLER_A:[^ ]+]] = OpLoad {{[^ ]+}} %[[SAMPLER_PTR_A]]{{( |$)}}
// ARRAYS: %[[PAIR_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_A]] %[[SAMPLER_A]]{{( |$)}}
// ARRAYS: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_A]] {{[^ ]+}} Lod %float_0{{( |$)}}
// ARRAYS-NOT: OpLabel
// ARRAYS: OpBranch %[[MERGE]]{{( |$)}}
// ARRAYS: %[[SECOND]] = OpLabel
// ARRAYS: %[[IMAGE_PTR_B:[^ ]+]] = OpAccessChain {{[^ ]+}} %secondTextures %[[SECOND_PAYLOAD]]{{( |$)}}
// ARRAYS: %[[SAMPLER_PTR_B:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %uint_0{{( |$)}}
// ARRAYS: %[[IMAGE_B:[^ ]+]] = OpLoad {{[^ ]+}} %[[IMAGE_PTR_B]]{{( |$)}}
// ARRAYS: %[[SAMPLER_B:[^ ]+]] = OpLoad {{[^ ]+}} %[[SAMPLER_PTR_B]]{{( |$)}}
// ARRAYS: %[[PAIR_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_B]] %[[SAMPLER_B]]{{( |$)}}
// ARRAYS: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_B]] {{[^ ]+}} Lod %float_0{{( |$)}}
// ARRAYS-NOT: OpLabel
// ARRAYS: OpBranch %[[MERGE]]{{( |$)}}
// ARRAYS: %[[MERGE]] = OpLabel
// GLSL_ARRAYS: bool [[CONDITION:[^ ]+]] = (uv_0.x) > 0.5;
// GLSL_ARRAYS: uint [[FIRST:[^ ]+]] = [[CONDITION]] ? Parameters_0.index_0 % 4U : 0U;
// GLSL_ARRAYS: uint [[SECOND:[^ ]+]] = [[CONDITION]] ? 0U : Parameters_0.otherIndex_0 % 4U;
// GLSL_ARRAYS: if([[CONDITION]])
// GLSL_ARRAYS-NEXT: {
// GLSL_ARRAYS-NOT: }
// GLSL_ARRAYS: textureLod(sampler2D(firstTextures_0[nonuniformEXT([[FIRST]])],samplers_0[0U]),
// GLSL_ARRAYS-NOT: textureLod
// GLSL_ARRAYS-NOT: texelFetch
// GLSL_ARRAYS: }
// GLSL_ARRAYS-NEXT: else
// GLSL_ARRAYS-NEXT: {
// GLSL_ARRAYS-NOT: }
// GLSL_ARRAYS: textureLod(sampler2D(secondTextures_0[nonuniformEXT([[SECOND]])],samplers_0[0U]),
// GLSL_ARRAYS-NOT: textureLod
// GLSL_ARRAYS-NOT: texelFetch
// GLSL_ARRAYS: }
float4 differentArrays(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> selected = select(uv.x > 0.5,
        firstTextures[index % 4], secondTextures[otherIndex % 4]);
    return selected.SampleLevel(samplers[0], uv, 0);
}

// The two coordinates of each nested array access must keep their distinct index expressions.
// GLSL_NESTED: bool [[CONDITION:[^ ]+]] = (uv_0.x) > 0.5;
// GLSL_NESTED: uint [[FIRST_ROW:[^ ]+]] = [[CONDITION]] ? Parameters_0.index_0 % 2U : 0U;
// GLSL_NESTED: uint [[FIRST_COLUMN:[^ ]+]] = [[CONDITION]] ? Parameters_0.otherIndex_0 % 4U : 0U;
// GLSL_NESTED: uint [[SECOND_ROW:[^ ]+]] = [[CONDITION]] ? 0U : Parameters_0.otherIndex_0 % 2U;
// GLSL_NESTED: uint [[SECOND_COLUMN:[^ ]+]] = [[CONDITION]] ? 0U : Parameters_0.index_0 % 4U;
// GLSL_NESTED: if([[CONDITION]])
// GLSL_NESTED-NEXT: {
// GLSL_NESTED-NOT: }
// GLSL_NESTED: textureLod(sampler2D(textureRows_0[nonuniformEXT([[FIRST_ROW]])][nonuniformEXT([[FIRST_COLUMN]])],samplers_0[0U]),
// GLSL_NESTED-NOT: textureLod
// GLSL_NESTED-NOT: texelFetch
// GLSL_NESTED: }
// GLSL_NESTED-NEXT: else
// GLSL_NESTED-NEXT: {
// GLSL_NESTED-NOT: }
// GLSL_NESTED: textureLod(sampler2D(otherRows_0[nonuniformEXT([[SECOND_ROW]])][nonuniformEXT([[SECOND_COLUMN]])],samplers_0[0U]),
// GLSL_NESTED-NOT: textureLod
// GLSL_NESTED-NOT: texelFetch
// GLSL_NESTED: }
float4 nestedArrays(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> selected = select(uv.x > 0.5,
        textureRows[index % 2][otherIndex % 4],
        otherRows[otherIndex % 2][index % 4]);
    return selected.SampleLevel(samplers[0], uv, 0);
}

// Selecting a whole array preserves its declaration choice while the later subscript stays
// the same ordinary value in both reconstruction branches.
// SELECTED_ARRAY: %[[CONDITION:[^ ]+]] = {{OpLogicalNot|OpFUnordLessThanEqual}} %bool{{( |$)}}
// SELECTED_ARRAY: %[[FIELD:[^ ]+]] = OpAccessChain {{[^ ]+}} %Parameters %int_0{{( |$)}}
// SELECTED_ARRAY: %[[INPUT:[^ ]+]] = OpLoad %uint %[[FIELD]]{{( |$)}}
// SELECTED_ARRAY: %[[INDEX:[^ ]+]] = OpUMod %uint %[[INPUT]] %uint_4{{( |$)}}
// SELECTED_ARRAY: OpSelectionMerge %[[MERGE:[^ ]+]] None
// SELECTED_ARRAY: OpBranchConditional %[[CONDITION]] %[[SECOND:[^ ]+]] %[[FIRST:[^ ]+]]{{( |$)}}
// SELECTED_ARRAY: %[[SECOND]] = OpLabel
// SELECTED_ARRAY: %[[IMAGE_PTR_B:[^ ]+]] = OpAccessChain {{[^ ]+}} %secondTextures %[[INDEX]]{{( |$)}}
// SELECTED_ARRAY: %[[SAMPLER_PTR_B:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %uint_0{{( |$)}}
// SELECTED_ARRAY: %[[IMAGE_B:[^ ]+]] = OpLoad {{[^ ]+}} %[[IMAGE_PTR_B]]{{( |$)}}
// SELECTED_ARRAY: %[[SAMPLER_B:[^ ]+]] = OpLoad {{[^ ]+}} %[[SAMPLER_PTR_B]]{{( |$)}}
// SELECTED_ARRAY: %[[PAIR_B:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_B]] %[[SAMPLER_B]]{{( |$)}}
// SELECTED_ARRAY: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_B]] {{[^ ]+}} Lod %float_0{{( |$)}}
// SELECTED_ARRAY-NOT: OpLabel
// SELECTED_ARRAY: OpBranch %[[MERGE]]{{( |$)}}
// SELECTED_ARRAY: %[[FIRST]] = OpLabel
// SELECTED_ARRAY: %[[IMAGE_PTR_A:[^ ]+]] = OpAccessChain {{[^ ]+}} %firstTextures %[[INDEX]]{{( |$)}}
// SELECTED_ARRAY: %[[SAMPLER_PTR_A:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %uint_0{{( |$)}}
// SELECTED_ARRAY: %[[IMAGE_A:[^ ]+]] = OpLoad {{[^ ]+}} %[[IMAGE_PTR_A]]{{( |$)}}
// SELECTED_ARRAY: %[[SAMPLER_A:[^ ]+]] = OpLoad {{[^ ]+}} %[[SAMPLER_PTR_A]]{{( |$)}}
// SELECTED_ARRAY: %[[PAIR_A:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE_A]] %[[SAMPLER_A]]{{( |$)}}
// SELECTED_ARRAY: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR_A]] {{[^ ]+}} Lod %float_0{{( |$)}}
// SELECTED_ARRAY-NOT: OpLabel
// SELECTED_ARRAY: OpBranch %[[MERGE]]{{( |$)}}
// SELECTED_ARRAY: %[[MERGE]] = OpLabel
// GLSL_ARRAY: uint [[INDEX:[^ ]+]] = Parameters_0.index_0 % 4U;
// GLSL_ARRAY: if(!((uv_0.x) > 0.5))
// GLSL_ARRAY-NEXT: {
// GLSL_ARRAY-NOT: }
// GLSL_ARRAY: textureLod(sampler2D(secondTextures_0{{\[}}[[INDEX]]{{\]}},samplers_0[0U]),
// GLSL_ARRAY-NOT: textureLod
// GLSL_ARRAY-NOT: texelFetch
// GLSL_ARRAY: }
// GLSL_ARRAY-NEXT: else
// GLSL_ARRAY-NEXT: {
// GLSL_ARRAY-NOT: }
// GLSL_ARRAY: textureLod(sampler2D(firstTextures_0{{\[}}[[INDEX]]{{\]}},samplers_0[0U]),
// GLSL_ARRAY-NOT: textureLod
// GLSL_ARRAY-NOT: texelFetch
// GLSL_ARRAY: }
float4 selectedArray(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> textures[4] = uv.x > 0.5 ? firstTextures : secondTextures;
    return textures[index % 4].SampleLevel(samplers[0], uv, 0);
}

// The selected rows have different declarations and row indices. We preserve that choice
// independently of the column index computed after the row selection.
// GLSL_ROW: bool [[CONDITION:[^ ]+]] = (uv_0.x) > 0.5;
// GLSL_ROW: if([[CONDITION]])
// GLSL_ROW-NEXT: {
// GLSL_ROW-NOT: }
// GLSL_ROW: [[ROW:[^ ]+]] = 0U;
// GLSL_ROW: }
// GLSL_ROW-NEXT: else
// GLSL_ROW-NEXT: {
// GLSL_ROW-NOT: }
// GLSL_ROW: [[ROW]] = 1U;
// GLSL_ROW: }
// GLSL_ROW: uint [[COLUMN:[^ ]+]] = Parameters_0.index_0 % 4U;
// GLSL_ROW: if(![[CONDITION]])
// GLSL_ROW-NEXT: {
// GLSL_ROW-NOT: }
// GLSL_ROW: textureLod(sampler2D(otherRows_0[nonuniformEXT([[ROW]])]{{\[}}[[COLUMN]]{{\]}},samplers_0[0U]),
// GLSL_ROW-NOT: textureLod
// GLSL_ROW-NOT: texelFetch
// GLSL_ROW: }
// GLSL_ROW-NEXT: else
// GLSL_ROW-NEXT: {
// GLSL_ROW-NOT: }
// GLSL_ROW: textureLod(sampler2D(textureRows_0[nonuniformEXT(0U)]{{\[}}[[COLUMN]]{{\]}},samplers_0[0U]),
// GLSL_ROW-NOT: textureLod
// GLSL_ROW-NOT: texelFetch
// GLSL_ROW: }
float4 selectedRow(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> row[4] = uv.x > 0.5 ? textureRows[0] : otherRows[1];
    return row[index % 4].SampleLevel(samplers[0], uv, 0);
}

// The heap and declared sampler array use distinct index payloads. Each branch must load
// its sampler from the corresponding declaration and pass that sampler to the actual sample.
// HEAP: %selected = OpFOrdGreaterThan %bool{{( |$)}}
// HEAP: %[[HEAP_FIELD:[^ ]+]] = OpAccessChain {{[^ ]+}} %Parameters %int_0{{( |$)}}
// HEAP: %[[HEAP_INPUT:[^ ]+]] = OpLoad %uint %[[HEAP_FIELD]]{{( |$)}}
// HEAP: %[[ARRAY_FIELD:[^ ]+]] = OpAccessChain {{[^ ]+}} %Parameters %int_1{{( |$)}}
// HEAP: %[[ARRAY_INPUT:[^ ]+]] = OpLoad %uint %[[ARRAY_FIELD]]{{( |$)}}
// HEAP: %[[ARRAY_MOD:[^ ]+]] = OpUMod %uint %[[ARRAY_INPUT]] %uint_4{{( |$)}}
// HEAP: %[[HEAP_PAYLOAD:[^ ]+]] = OpSelect %uint %selected %[[HEAP_INPUT]] %uint_0{{( |$)}}
// HEAP: %[[ARRAY_PAYLOAD:[^ ]+]] = OpSelect %uint %selected %uint_0 %[[ARRAY_MOD]]{{( |$)}}
// HEAP: OpSelectionMerge %[[MERGE:[^ ]+]] None
// HEAP: OpBranchConditional %selected %[[HEAP:[^ ]+]] %[[ARRAY:[^ ]+]]{{( |$)}}
// HEAP: %[[HEAP]] = OpLabel
// HEAP: %[[HEAP_PTR:[^ ]+]] = OpUntypedAccessChainKHR {{[^ ]+}} {{[^ ]+}} %slang_samplerHeap %[[HEAP_PAYLOAD]]{{( |$)}}
// HEAP: %[[HEAP_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[HEAP_PTR]]{{( |$)}}
// HEAP: %[[HEAP_TEXTURE:[^ ]+]] = OpLoad {{[^ ]+}} %singleTexture{{( |$)}}
// HEAP: %[[HEAP_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[HEAP_TEXTURE]] %[[HEAP_SAMPLER]]{{( |$)}}
// HEAP: OpImageSampleExplicitLod {{[^ ]+}} %[[HEAP_PAIR]]{{( |$)}}
// HEAP-NOT: OpLabel
// HEAP: OpBranch %[[MERGE]]{{( |$)}}
// HEAP: %[[ARRAY]] = OpLabel
// HEAP: %[[ARRAY_PTR:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[ARRAY_PAYLOAD]]{{( |$)}}
// HEAP: %[[ARRAY_SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[ARRAY_PTR]]{{( |$)}}
// HEAP: %[[ARRAY_TEXTURE:[^ ]+]] = OpLoad {{[^ ]+}} %singleTexture{{( |$)}}
// HEAP: %[[ARRAY_PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[ARRAY_TEXTURE]] %[[ARRAY_SAMPLER]]{{( |$)}}
// HEAP: OpImageSampleExplicitLod {{[^ ]+}} %[[ARRAY_PAIR]]{{( |$)}}
// HEAP-NOT: OpLabel
// HEAP: OpBranch %[[MERGE]]{{( |$)}}
// HEAP: %[[MERGE]] = OpLabel
float4 heapOrArray(float2 uv : TEXCOORD) : SV_Target
{
    SamplerState selected = select(uv.x > 0.5,
        (SamplerState)SamplerDescriptorHeap[index], samplers[otherIndex % 4]);
    return singleTexture.SampleLevel(selected, uv, 0);
}
