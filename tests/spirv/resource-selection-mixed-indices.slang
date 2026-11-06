//TEST:SIMPLE(filecheck=PHI): -target spirv-asm -stage fragment -entry mixedPhi -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=PHI): -target spirv-asm -stage fragment -entry mixedPhi -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry mixedPhi -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SELECT): -target spirv-asm -stage fragment -entry mixedSelect -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SELECT): -target spirv-asm -stage fragment -entry mixedSelect -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry mixedSelect -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=LOOP): -target spirv-asm -stage fragment -entry mixedLoop -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=LOOP): -target spirv-asm -stage fragment -entry mixedLoop -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry mixedLoop -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WIDE): -target spirv-asm -stage fragment -entry mixedWidth -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=WIDE): -target spirv-asm -stage fragment -entry mixedWidth -validate-ir -validate-ir-detailed

// Check that selecting resources whose array subscripts have different integer types merges
// those indices without leaving resource-valued phis/selects or narrowing a wider index.
// The GLSL check applies to all three GLSL runs.
// GLSL: nonuniformEXT
SamplerState samplers[8];
Texture2D<float4> texture;
cbuffer Ubo { uint i; int j; uint count; uint64_t wide; };

// PHI: OpTypePointer Function %uint
// PHI: %[[PHI_INDEX:[^ ]+]] = OpLoad %uint %s
// PHI: OpAccessChain {{.*}} %samplers %[[PHI_INDEX]]
// PHI: OpImageSampleExplicitLod
float4 mixedPhi(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState s = uv.x < 0.5 ? samplers[0] : samplers[i];
    return texture.SampleLevel(s, uv, 0);
}

// SELECT: %[[SIGNED:[^ ]+]] = OpBitcast %uint
// SELECT: %s = OpSelect %uint {{.*}} %[[SIGNED]]
// SELECT: OpAccessChain {{.*}} %samplers %s
// SELECT: OpImageSampleExplicitLod
float4 mixedSelect(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState s = select(uv.x < 0.5, samplers[j], samplers[i]);
    return texture.SampleLevel(s, uv, 0);
}

// LOOP: OpTypePointer Function %uint
// LOOP: OpLoopMerge
// LOOP: %[[LOOP_INDEX:[^ ]+]] = OpLoad %uint %s
// LOOP: OpAccessChain {{.*}} %samplers %[[LOOP_INDEX]]
// LOOP: OpImageSampleExplicitLod
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
// WIDE: %[[WIDENED:[^ ]+]] = OpUConvert %[[WIDE_TYPE]]
// WIDE: %inner = OpSelect %[[WIDE_TYPE]] {{.*}} %[[WIDENED]]
// WIDE: %s = OpSelect %[[WIDE_TYPE]] {{.*}} %inner
// WIDE: OpAccessChain {{.*}} %samplers %s
// WIDE: OpImageSampleExplicitLod
float4 mixedWidth(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState inner = select(uv.x < 0.5, samplers[i], samplers[j]);
    SamplerState s = select(uv.y < 0.5, inner, samplers[wide]);
    return texture.SampleLevel(s, uv, 0);
}
