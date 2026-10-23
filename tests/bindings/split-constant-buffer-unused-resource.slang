// Entry-point `uniform` parameters that mix ordinary data with resources are collected into one
// constant buffer, which resource legalization splits into a data-only buffer plus one parameter
// per resource (shader-slang/slang#13789). The data-only buffer must not shift any emitted
// register or binding. The HLSL run checks the buffer's own `b0` and the used resource; the SPIR-V
// run checks the resource bindings, since the buffer there is a push-constant block with no
// binding. The metadata itself is checked by `isParameterLocationUsedMixedConstantBuffer`.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_5_0 -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry main -stage compute

// HLSL: cbuffer entryPointParams_{{[0-9]+}} : register(b0)
// HLSL: StructuredBuffer<float4 > entryPointParams_usedBuf_{{[0-9]+}} : register(t0);
// HLSL-NOT: unusedBuf

// SPIRV-DAG: OpDecorate %Output Binding 0
// SPIRV-DAG: OpDecorate %entryPointParams_usedBuf Binding 1
// SPIRV-NOT: unusedBuf

RWStructuredBuffer<float4> Output;

[numthreads(64, 1, 1)]
void main(
    uniform float4 tint,
    uniform StructuredBuffer<float4> usedBuf,
    uniform StructuredBuffer<float4> unusedBuf,
    uint3 tid: SV_DispatchThreadID)
{
    Output[tid.x] = usedBuf[tid.x] * tint;
}
