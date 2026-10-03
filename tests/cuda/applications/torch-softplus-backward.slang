// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Reduced from SlangPy's softplus activation gradient-parity backward shader.
// Consider y = log(1 + exp(x)), with y = x when x > 20. A scalar-only backward call compiles.
// Retaining pointer-backed input/output tensor descriptors and their custom load/store
// derivatives makes standalone NVVM O3 compilation time out; NVVM O0 compiles and assembles.
// The application also times out during backward; a shared native root cause is not yet proven.
// Keep the branch, custom derivatives and observable gradient accumulation when reducing further.
// Desired execution: add outputGradient * (x > 20 ? 1 : 1/(1+exp(-x))) to inputGradient.
// Run with a process timeout. A timeout is an unresolved failure, never an expected pass.

struct Context
{
    uint3 threadID;
    int callID[2];
}
struct Tensor
{
    float* data;
    uint shape[2];
    uint strides[2];
    uint offset;
    uint index(Context context)
    {
        return offset + context.callID[0] * strides[0] + context.callID[1] * strides[1];
    }
}
struct InputTensor
{
    Tensor primal;
    Tensor gradient;
}
struct OutputTensor
{
    Tensor primal;
    Tensor gradient;
}
void loadBackward(InputTensor tensor, Context context, float gradient)
{
    float old;
    InterlockedAdd(tensor.gradient.data[tensor.gradient.index(context)], gradient, old);
}
[Differentiable]
[BackwardDerivative(loadBackward)]
float load(no_diff InputTensor tensor, no_diff Context context)
{
    return tensor.primal.data[tensor.primal.index(context)];
}
void storeBackward(OutputTensor tensor, Context context, inout DifferentialPair<float> value)
{
    value = diffPair(value.p, tensor.gradient.data[tensor.gradient.index(context)]);
}
[Differentiable]
[BackwardDerivative(storeBackward)]
void store(no_diff OutputTensor tensor, no_diff Context context, float value)
{
    tensor.primal.data[tensor.primal.index(context)] = detach(value);
}
[Differentiable]
float softplus(float x)
{
    if (x > 20.0)
        return x;
    return log(1.0 + exp(x));
}
[Differentiable]
void trampoline(no_diff Context context, no_diff InputTensor x, no_diff OutputTensor y)
{
    float input = load(x, context);
    float result = softplus(input);
    store(y, context, result);
}
[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uniform InputTensor x, uniform OutputTensor y)
{
    Context context = {tid, {int(tid.x), int(tid.y)}};
    bwd_diff(trampoline)(context, x, y);
}

// Scalar differentiation is a control for the tensor custom-derivative wrapper.
[shader("compute")]
[numthreads(1, 1, 1)]
void scalarControl(uniform float value, uniform RWStructuredBuffer<float> result)
{
    DifferentialPair<float> pair = diffPair(value, 0.0);
    bwd_diff(softplus)(pair, 1.0);
    result[0] = pair.d;
}
