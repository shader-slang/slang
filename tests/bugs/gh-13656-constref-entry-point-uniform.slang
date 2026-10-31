// An entry-point `uniform ConstantBuffer<P>` parameter passed to a
// `__constref P` parameter used to crash the compiler
// (https://github.com/shader-slang/slang/issues/13656).

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute

// SPIRV: OpEntryPoint GLCompute %computeMain "main" {{.*}}%entryPointParams_params
// SPIRV: %entryPointParams_params = OpVariable %{{.*}} Uniform

// HLSL: cbuffer entryPointParams_params
// HLSL: void computeMain(

struct Params
{
    float4 color;
};

RWStructuredBuffer<float4> outputBuffer;

float4 getColor(__constref Params p)
{
    return p.color;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uniform ConstantBuffer<Params> params)
{
    outputBuffer[tid.x] = getColor(params);
}
