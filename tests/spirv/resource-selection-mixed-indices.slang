//TEST:SIMPLE(filecheck=PHI): -target spirv-asm -stage fragment -entry mixedPhi -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PHI): -target spirv-asm -stage fragment -entry mixedPhi -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry mixedPhi -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SELECT): -target spirv-asm -stage fragment -entry mixedSelect -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SELECT): -target spirv-asm -stage fragment -entry mixedSelect -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry mixedSelect -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=LOOP): -target spirv-asm -stage fragment -entry mixedLoop -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=LOOP-OPT): -target spirv-asm -stage fragment -entry mixedLoop -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry mixedLoop -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WIDE): -target spirv-asm -stage fragment -entry mixedWidth -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WIDE): -target spirv-asm -stage fragment -entry mixedWidth -validate-ir -validate-ir-detailed -O1

// Check that selecting resources whose array subscripts have different integer types merges
// those indices without leaving resource-valued phis/selects or narrowing a wider index.
// The GLSL check applies to all three GLSL runs.
// GLSL: nonuniformEXT
SamplerState samplers[8];
Texture2D<float4> texture;
cbuffer Ubo { uint i; int j; uint count; uint64_t wide; };

// PHI: %[[UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// PHI: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// PHI: %[[CONDITION:[^ ]+]] = OpFOrdLessThan %bool %[[X]] %float_0_5{{( |$)}}
// PHI: OpSelectionMerge %[[MERGE:[^ ]+]] None
// PHI: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
// PHI: %[[THEN]] = OpLabel
// PHI: OpBranch %[[MERGE]]{{( |$)}}
// PHI: %[[ELSE]] = OpLabel
// PHI: %[[INPUT_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_0{{( |$)}}
// PHI: %[[INPUT:[^ ]+]] = OpLoad %uint %[[INPUT_POINTER]]{{( |$)}}
// PHI: OpBranch %[[MERGE]]{{( |$)}}
// PHI: %[[MERGE]] = OpLabel
// PHI: %[[PHI_INDEX:[^ ]+]] = OpPhi %uint %uint_0 %[[THEN]] %[[INPUT]] %[[ELSE]]{{( |$)}}
// PHI: %[[POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[PHI_INDEX]]{{( |$)}}
// PHI: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[POINTER]]{{( |$)}}
// PHI: %[[IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// PHI: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE]] %[[SAMPLER]]{{( |$)}}
// PHI: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}
float4 mixedPhi(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState s = uv.x < 0.5 ? samplers[0] : samplers[i];
    return texture.SampleLevel(s, uv, 0);
}

// SELECT: %[[UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// SELECT: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// SELECT: %[[CONDITION:[^ ]+]] = OpFOrdLessThan %bool %[[X]] %float_0_5{{( |$)}}
// SELECT: %[[J_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_1{{( |$)}}
// SELECT: %[[J:[^ ]+]] = OpLoad %int %[[J_POINTER]]{{( |$)}}
// SELECT: %[[SIGNED:[^ ]+]] = OpBitcast %uint %[[J]]{{( |$)}}
// SELECT: %[[I_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_0{{( |$)}}
// SELECT: %[[I:[^ ]+]] = OpLoad %uint %[[I_POINTER]]{{( |$)}}
// SELECT: %s = OpSelect %uint %[[CONDITION]] %[[SIGNED]] %[[I]]{{( |$)}}
// SELECT: %[[POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %s{{( |$)}}
// SELECT: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[POINTER]]{{( |$)}}
// SELECT: %[[IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// SELECT: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE]] %[[SAMPLER]]{{( |$)}}
// SELECT: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}
float4 mixedSelect(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState s = select(uv.x < 0.5, samplers[j], samplers[i]);
    return texture.SampleLevel(s, uv, 0);
}

// The header phi preserves index zero for an empty loop and the last selected index otherwise.
// Follow both the index and induction value around the back edge before checking the final sample.
// LOOP: OpFunction
// LOOP: %[[ENTRY:[^ ]+]] = OpLabel
// LOOP: OpBranch %[[HEADER:[^ ]+]]{{( |$)}}
// LOOP: %[[HEADER]] = OpLabel
// LOOP: %[[CARRIED_INDEX:[^ ]+]] = OpPhi %uint %[[NEXT_INDEX:[^ ]+]] %[[BACKEDGE:[^ ]+]] %uint_0 %[[ENTRY]]{{( |$)}}
// LOOP: %[[K:[^ ]+]] = OpPhi %uint %[[NEXT_K:[^ ]+]] %[[BACKEDGE]] %uint_0 %[[ENTRY]]{{( |$)}}
// LOOP: OpLoopMerge %[[EXIT:[^ ]+]] %[[BACKEDGE]] None
// LOOP: %[[UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// LOOP: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// LOOP: %[[CONDITION:[^ ]+]] = OpFOrdLessThan %bool %[[X]] %float_0_5{{( |$)}}
// LOOP: OpSelectionMerge %[[MERGE:[^ ]+]] None
// LOOP: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
// LOOP: %[[THEN]] = OpLabel
// LOOP: %[[J_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_1{{( |$)}}
// LOOP: %[[J:[^ ]+]] = OpLoad %int %[[J_POINTER]]{{( |$)}}
// LOOP: %[[SIGNED_K:[^ ]+]] = OpBitcast %int %[[K]]{{( |$)}}
// LOOP: %[[SUM:[^ ]+]] = OpIAdd %int %[[J]] %[[SIGNED_K]]{{( |$)}}
// LOOP: %[[CONVERTED:[^ ]+]] = OpBitcast %uint %[[SUM]]{{( |$)}}
// LOOP: OpBranch %[[MERGE]]{{( |$)}}
// LOOP: %[[ELSE]] = OpLabel
// LOOP: %[[I_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_0{{( |$)}}
// LOOP: %[[I:[^ ]+]] = OpLoad %uint %[[I_POINTER]]{{( |$)}}
// LOOP: OpBranch %[[MERGE]]{{( |$)}}
// LOOP: %[[MERGE]] = OpLabel
// LOOP: %[[NEXT_INDEX]] = OpPhi %uint %[[CONVERTED]] %[[THEN]] %[[I]] %[[ELSE]]{{( |$)}}
// LOOP: %[[NEXT_K]] = OpIAdd %uint %[[K]] %uint_1{{( |$)}}
// LOOP: OpBranch %[[BACKEDGE]]{{( |$)}}
// LOOP: %[[BACKEDGE]] = OpLabel
// LOOP: OpBranch %[[HEADER]]{{( |$)}}
// LOOP: %[[EXIT]] = OpLabel
// LOOP: %[[POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[CARRIED_INDEX]]{{( |$)}}
// LOOP: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[POINTER]]{{( |$)}}
// LOOP: %[[IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// LOOP: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE]] %[[SAMPLER]]{{( |$)}}
// LOOP: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}
// SPIR-V optimization places the false arm before the true arm. We check the same input values
// and common merge in that order rather than assuming the text order determines the selected arm.
// LOOP-OPT: OpFunction
// LOOP-OPT: %[[ENTRY:[^ ]+]] = OpLabel
// LOOP-OPT: OpBranch %[[HEADER:[^ ]+]]{{( |$)}}
// LOOP-OPT: %[[HEADER]] = OpLabel
// LOOP-OPT: %[[CARRIED_INDEX:[^ ]+]] = OpPhi %uint %[[NEXT_INDEX:[^ ]+]] %[[BACKEDGE:[^ ]+]] %uint_0 %[[ENTRY]]{{( |$)}}
// LOOP-OPT: %[[K:[^ ]+]] = OpPhi %uint %[[NEXT_K:[^ ]+]] %[[BACKEDGE]] %uint_0 %[[ENTRY]]{{( |$)}}
// LOOP-OPT: OpLoopMerge %[[EXIT:[^ ]+]] %[[BACKEDGE]] None
// LOOP-OPT: %[[UV:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// LOOP-OPT: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV]] 0
// LOOP-OPT: %[[CONDITION:[^ ]+]] = OpFOrdLessThan %bool %[[X]] %float_0_5{{( |$)}}
// LOOP-OPT: OpSelectionMerge %[[MERGE:[^ ]+]] None
// LOOP-OPT: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
// LOOP-OPT: %[[ELSE]] = OpLabel
// LOOP-OPT: %[[I_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_0{{( |$)}}
// LOOP-OPT: %[[I:[^ ]+]] = OpLoad %uint %[[I_POINTER]]{{( |$)}}
// LOOP-OPT: OpBranch %[[MERGE]]{{( |$)}}
// LOOP-OPT: %[[THEN]] = OpLabel
// LOOP-OPT: %[[J_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_1{{( |$)}}
// LOOP-OPT: %[[J:[^ ]+]] = OpLoad %int %[[J_POINTER]]{{( |$)}}
// LOOP-OPT: %[[SIGNED_K:[^ ]+]] = OpBitcast %int %[[K]]{{( |$)}}
// LOOP-OPT: %[[SUM:[^ ]+]] = OpIAdd %int %[[J]] %[[SIGNED_K]]{{( |$)}}
// LOOP-OPT: %[[CONVERTED:[^ ]+]] = OpBitcast %uint %[[SUM]]{{( |$)}}
// LOOP-OPT: OpBranch %[[MERGE]]{{( |$)}}
// LOOP-OPT: %[[MERGE]] = OpLabel
// LOOP-OPT: %[[NEXT_INDEX]] = OpPhi %uint %[[CONVERTED]] %[[THEN]] %[[I]] %[[ELSE]]{{( |$)}}
// LOOP-OPT: %[[NEXT_K]] = OpIAdd %uint %[[K]] %uint_1{{( |$)}}
// LOOP-OPT: OpBranch %[[BACKEDGE]]{{( |$)}}
// LOOP-OPT: %[[BACKEDGE]] = OpLabel
// LOOP-OPT: OpBranch %[[HEADER]]{{( |$)}}
// LOOP-OPT: %[[EXIT]] = OpLabel
// LOOP-OPT: %[[POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %[[CARRIED_INDEX]]{{( |$)}}
// LOOP-OPT: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[POINTER]]{{( |$)}}
// LOOP-OPT: %[[IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// LOOP-OPT: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE]] %[[SAMPLER]]{{( |$)}}
// LOOP-OPT: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}
float4 mixedLoop(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState s = samplers[0];
    for (uint k = 0; k < count; k++)
    {
        // The conversion must remain on this branch, after the signed index computation.
        // -validate-ir-detailed checks this through dominance validation.
        if (uv.x < 0.5)
            s = samplers[j + int(k)];
        else
            s = samplers[i];
    }
    return texture.SampleLevel(s, uv, 0);
}

// Both nested `select` instructions must use the widest index type, including indices converted
// before the inner choice. We check the generated representation without assuming a device supports
// 2^32 descriptors or 64-bit buffer addressing. In-bounds values of `wide` are 0..7.
// WIDE: %[[WIDE_TYPE:[^ ]+]] = OpTypeInt 64 0
// WIDE: %[[UV_X:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// WIDE: %[[X:[^ ]+]] = OpCompositeExtract %float %[[UV_X]] 0
// WIDE: %[[INNER_CONDITION:[^ ]+]] = OpFOrdLessThan %bool %[[X]] %float_0_5{{( |$)}}
// WIDE: %[[I_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_0{{( |$)}}
// WIDE: %[[I:[^ ]+]] = OpLoad %uint %[[I_POINTER]]{{( |$)}}
// WIDE: %[[WIDENED:[^ ]+]] = OpUConvert %[[WIDE_TYPE]] %[[I]]{{( |$)}}
// WIDE: %[[J_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_1{{( |$)}}
// WIDE: %[[J:[^ ]+]] = OpLoad %int %[[J_POINTER]]{{( |$)}}
// WIDE: %[[SIGNED:[^ ]+]] = OpSConvert %[[WIDE_TYPE]] %[[J]]{{( |$)}}
// WIDE: %inner = OpSelect %[[WIDE_TYPE]] %[[INNER_CONDITION]] %[[WIDENED]] %[[SIGNED]]{{( |$)}}
// WIDE: %[[UV_Y:[^ ]+]] = OpLoad %v2float %uv{{( |$)}}
// WIDE: %[[Y:[^ ]+]] = OpCompositeExtract %float %[[UV_Y]] 1
// WIDE: %[[OUTER_CONDITION:[^ ]+]] = OpFOrdLessThan %bool %[[Y]] %float_0_5{{( |$)}}
// WIDE: %[[WIDE_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Ubo %int_3{{( |$)}}
// WIDE: %[[WIDE_INDEX:[^ ]+]] = OpLoad %[[WIDE_TYPE]] %[[WIDE_POINTER]]{{( |$)}}
// WIDE: %s = OpSelect %[[WIDE_TYPE]] %[[OUTER_CONDITION]] %inner %[[WIDE_INDEX]]{{( |$)}}
// WIDE: %[[POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %samplers %s{{( |$)}}
// WIDE: %[[SAMPLER:[^ ]+]] = OpLoad {{[^ ]+}} %[[POINTER]]{{( |$)}}
// WIDE: %[[IMAGE:[^ ]+]] = OpLoad {{[^ ]+}} %texture{{( |$)}}
// WIDE: %[[PAIR:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[IMAGE]] %[[SAMPLER]]{{( |$)}}
// WIDE: OpImageSampleExplicitLod {{[^ ]+}} %[[PAIR]]{{( |$)}}
float4 mixedWidth(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState inner = select(uv.x < 0.5, samplers[i], samplers[j]);
    SamplerState s = select(uv.y < 0.5, inner, samplers[wide]);
    return texture.SampleLevel(s, uv, 0);
}
