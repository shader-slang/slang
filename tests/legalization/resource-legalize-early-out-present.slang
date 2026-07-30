// Regression for shader-slang/slang#11917: `legalizeResourceTypes` gained an entry-time early-out
// that skips the pass when the presence scan (`hasResourceLegalizationWork`) finds no resource type.
// A struct with a resource field must still be split, so the scan must detect the resource and run
// the pass. The primary guard is that compilation *succeeds*: a wrongly-skipped resource pass leaves
// the opaque handle inside a `struct`, and SPIR-V type translation then aborts with
// `E56004: opaque type in local variable` before any check runs. The OpTypeImage check below is a
// secondary assertion that the texture reached emission as a standalone image once the split ran.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain

struct Material
{
    float4 color;
    Texture2D tex;
    SamplerState samp;
};

RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uniform Material m)
{
    outputBuffer[tid.x] = m.color + m.tex.SampleLevel(m.samp, float2(0, 0), 0);
}

// CHECK: OpTypeImage
