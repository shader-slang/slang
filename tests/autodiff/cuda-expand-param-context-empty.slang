//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -validate-ir

// A custom derivative with no primal parameters has an empty captured-parameter
// context. Expanding it must preserve the output seed and the derivative call.
// CUDA-DAG: __device__ void backwardEmpty(float derivative_{{[0-9]+}})
// CUDA-DAG: accumulateGlobalGradient(

[__NoSideEffect]
__target_intrinsic(cuda) float readGlobalValue();

__target_intrinsic(cuda) void accumulateGlobalGradient(float derivative);

[BackwardDerivative(accumulateGlobalGradient)]
float readValue()
{
    return readGlobalValue();
}

[Differentiable]
float objective()
{
    return readValue() * readValue();
}

[CudaDeviceExport]
void backwardEmpty(float derivative)
{
    bwd_diff(objective)(derivative);
}
