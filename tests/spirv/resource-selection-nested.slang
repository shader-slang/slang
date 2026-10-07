//TEST:SIMPLE(filecheck=SELECT): -target spirv-asm -stage fragment -entry nested -O0 -validate-ir
//TEST:SIMPLE(filecheck=SELECT): -target spirv-asm -stage fragment -entry nested -validate-ir
//TEST:SIMPLE(filecheck=LOOP): -target spirv-asm -stage fragment -entry swap -O0 -validate-ir
//TEST:SIMPLE(filecheck=LOOP): -target spirv-asm -stage fragment -entry swap -validate-ir

// Check that nested sampler selects become integer selects with correctly typed nonuniform indices.
// Check that swapping samplers in a loop uses integer state instead of local sampler variables.

// SELECT: %a = OpSelect %int
// SELECT: %b = OpSelect %int
// SELECT: %selected = OpSelect %int {{.*}} %a %b
// SELECT: OpAccessChain {{.*}} %samplers %selected
// SELECT: OpImageSampleExplicitLod
// LOOP: OpTypePointer Function %int
// LOOP: OpLoopMerge
// LOOP: OpImageSampleExplicitLod
// LOOP: OpImageSampleExplicitLod

SamplerState samplers[8];
Texture2D<float4> texture;
cbuffer Ubo { uint count; };

// Folding the outer select reads the nonuniform indices emitted for the inner selects.
// Their wrappers must retain the integer operand type, rather than type_t.
float4 nested(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState a = select(uv.x > 0.5, samplers[0], samplers[1]);
    SamplerState b = select(uv.y > 0.5, samplers[2], samplers[3]);
    SamplerState selected = select(uv.x > uv.y, a, b);
    return texture.SampleLevel(selected, uv, 0);
}

float4 swap(float2 uv : TEXCOORD) : SV_TARGET
{
    SamplerState a = samplers[0];
    SamplerState b = samplers[1];
    for (uint k = 0; k < count; k++)
    {
        SamplerState temporary = a;
        a = b;
        b = temporary;
    }
    return texture.SampleLevel(a, uv, 0) + texture.SampleLevel(b, uv, 0);
}
