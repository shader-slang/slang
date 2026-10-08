// Regression for shader-slang/slang#11917: a struct with a resource field must be split by
// `legalizeResourceTypes` so the resource is pulled out of the aggregate. This guards that the
// shared type-legalizer's worklist rework (a cheaper per-batch scratch-bit reset) preserves that
// behavior. The primary guard is that compilation *succeeds*: if the split were lost, the opaque
// handle would remain inside a `struct` and SPIR-V type translation would abort with
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
