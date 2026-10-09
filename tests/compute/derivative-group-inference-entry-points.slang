//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry linearMain -entry quadMain -O0
//TEST:SIMPLE(filecheck=SPV_EXCLUSIVE): -target spirv-asm -entry linearMain -entry quadMain -O0
//TEST:SIMPLE(filecheck=GLSL_LINEAR): -target glsl -entry linearMain -O0
//TEST:SIMPLE(filecheck=GLSL_QUAD): -target glsl -entry quadMain -O0

// SPV-DAG: OpExecutionMode %linearMain DerivativeGroupLinearKHR
// SPV-DAG: OpExecutionMode %quadMain DerivativeGroupQuadsKHR

// Use a separate prefix so the negative checks cover the entire module.
// SPV_EXCLUSIVE-NOT: OpExecutionMode %linearMain DerivativeGroupQuadsKHR
// SPV_EXCLUSIVE-NOT: OpExecutionMode %quadMain DerivativeGroupLinearKHR

// GLSL_LINEAR: layout(derivative_group_linearNV) in;
// GLSL_LINEAR-NOT: derivative_group_
// GLSL_QUAD: layout(derivative_group_quadsNV) in;
// GLSL_QUAD-NOT: derivative_group_

RWStructuredBuffer<float> output;

[noinline]
float derivative(float value)
{
    return ddx(value);
}

[shader("compute")]
[numthreads(16, 1, 1)]
void linearMain(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = derivative(float(tid.x));
}

[shader("compute")]
[numthreads(2, 2, 1)]
void quadMain(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = derivative(float(tid.x));
}
