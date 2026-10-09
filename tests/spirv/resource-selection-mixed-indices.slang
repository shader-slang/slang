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

// Ordinary array subscripts retain their integer types. Selecting resources must merge those
// indices without leaving resource-valued phis/selects or narrowing a wider index.
SamplerState samplers[8];
Texture2D<float4> texture;
cbuffer Ubo { uint i; int j; uint count; uint64_t wide; };

// PHI: OpTypePointer Function %uint
// PHI: %[[PHI_INDEX:[^ ]+]] = OpLoad %uint %s
// PHI: OpAccessChain {{.*}} %samplers %[[PHI_INDEX]]
// PHI: OpImageSampleExplicitLod
// GLSL: nonuniformEXT
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
        if (uv.x < 0.5)
            s = samplers[j + int(k)];
        else
            s = samplers[i];
    }
    return texture.SampleLevel(s, uv, 0);
}

// The outer selection must widen the result of the inner selection, even though the latter
// has already been rewritten to an array access with a nonuniform index. This checks that
// the IR preserves a 64-bit descriptor index, not that a device supports 2^32 descriptors
// or that address calculations inside a buffer use 64 bits. In-bounds values of wide are 0..7.
// WIDE: %[[WIDE_TYPE:[^ ]+]] = OpTypeInt 64 0
// WIDE: %inner = OpSelect %uint
// WIDE: %[[WIDENED:[^ ]+]] = OpUConvert %[[WIDE_TYPE]] %inner
// WIDE: %s = OpSelect %[[WIDE_TYPE]] {{.*}} %[[WIDENED]]
// WIDE: OpAccessChain {{.*}} %samplers %s
// WIDE: OpImageSampleExplicitLod
float4 mixedWidth(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState inner = select(uv.x < 0.5, samplers[i], samplers[j]);
    SamplerState s = select(uv.y < 0.5, inner, samplers[wide]);
    return texture.SampleLevel(s, uv, 0);
}
