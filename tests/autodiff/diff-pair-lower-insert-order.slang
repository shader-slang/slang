// Regression for shader-slang/slang#12914. `-validate-ir-detailed` validates the IR
// after every pass, which is required to observe the transient mid-pipeline ordering
// violation. The RWStructuredBuffer write is load-bearing: it keeps the vector
// `lerp` (and its differential machinery) alive so the DifferentialPair type-lowering
// path is exercised.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -validate-ir-detailed

RWStructuredBuffer<float4> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    float3 a = float3(1, 2, 3);
    float3 r = lerp(a, a + 1.0f, 0.5f);
    buf[tid.x] = float4(r, 0.0f);
}

// CHECK: OpEntryPoint
