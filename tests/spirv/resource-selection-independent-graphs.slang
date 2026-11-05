//TEST:SIMPLE(filecheck=SAMPLE): -target spirv-asm -stage fragment -entry sharedChoices -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SAMPLE): -target spirv-asm -stage fragment -entry sharedChoices -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry sharedChoices -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=LOOP): -target spirv-asm -stage fragment -entry loop -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=LOOP): -target spirv-asm -stage fragment -entry loop -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry loop -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SAMPLE): -target spirv-asm -stage fragment -entry independent -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SAMPLE): -target spirv-asm -stage fragment -entry independent -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry independent -validate-ir -validate-ir-detailed

// We exercise resource choices that are shared, nested, or carried around a loop.
// The loop swaps two samplers and updates a third, so legalization must preserve several
// mutually dependent resource choices alongside the ordinary loop counter.
// SAMPLE: OpImageSampleExplicitLod
// LOOP: OpLoopMerge
// LOOP: OpImageSampleExplicitLod
// GLSL: textureLod

Texture2D<float4> textureA;
Texture2D<float4> textureB;
SamplerState samplerA;
SamplerState samplerB;
SamplerState samplerC;
cbuffer Parameters { uint count; };

// Two later choices use `first`, and sampling `first` also keeps a separate consumer live.
// We require legalization to preserve those shared dependencies rather than follow each path
// as an independent tree.
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
float4 independent(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(uv.x > 0.5, textureA, textureB);
    SamplerState sampler = select(uv.y > 0.5, samplerA, samplerB);
    return texture.SampleLevel(sampler, uv, 0);
}
