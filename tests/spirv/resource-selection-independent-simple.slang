//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -DMANUAL -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -DMANUAL -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=NO-TAG): -target spirv-asm -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=NO-TAG): -target spirv-asm -stage fragment -entry main -DMANUAL -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=NO-TAG): -target spirv-asm -stage fragment -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=NO-TAG): -target spirv-asm -stage fragment -entry main -DMANUAL -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -DMANUAL -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -DMANUAL -O0 -validate-ir -validate-ir-detailed

// We compare a selected sampler with the equivalent hand-written conditional.
// Both forms must branch on the original boolean and sample only the selected resource.
// The separate `NO-TAG` checks cover the complete module and reject switches, integer selects,
// and cancelling Boolean negations in the simple case.
// SPIRV: %[[CONDITION:[^ ]+]] = OpFOrdGreaterThan %bool
// SPIRV: OpBranchConditional %[[CONDITION]]
// SPIRV: OpImageSampleExplicitLod
// SPIRV: OpImageSampleExplicitLod
// NO-TAG-NOT: OpSwitch
// NO-TAG-NOT: OpSelect %int
// NO-TAG-NOT: OpSelect %uint
// NO-TAG-NOT: OpLogicalNot
// GLSL: if(
// GLSL: textureLod
// GLSL: textureLod

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
