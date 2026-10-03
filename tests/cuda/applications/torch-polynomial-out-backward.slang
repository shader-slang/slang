// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Reduced from SlangPy's polynomial_out backward wrapper and tensor/context helpers.
// Consider res = a * x * x + b * x + c written through an out parameter. SlangPy differentiates a
// wrapper that loads x and stores res through custom tensor derivatives. Context.map(), the
// out-parameter load, and tensor methods preserve the composition producing the layout rejection.
// A direct bwd_diff(polynomial) on scalar differential pairs compiles and loses this failure.
// The tensor descriptors contain device pointers, shape, strides and offset; no host package
// or generated module is needed. Native and Python bridge modes generate the same shader shape.
// Desired execution: add (2*a*x+b)*outputGradient to inputGradient at the mapped element.
// This corpus currently establishes compilation failure only, not that runtime contract.

public struct Context<let D : int>
{
    public uint3 threadID;
    public int callID[D];
    public Context<M> map<let M : int>(int mapping[M])
    {
        Context<M> result;
        result.threadID = threadID;
        [ForceUnroll]
        for (int i = 0; i < M; i++)
            result.callID[i] = callID[mapping[i]];
        return result;
    }
}
public struct Tensor<let D : int>
{
    public float* data;
    public uint shape[D];
    public uint strides[D];
    public uint offset;
    public uint index(int idx[D])
    {
        uint result = offset;
        [ForceUnroll]
        for (int i = 0; i < D; i++)
            result += idx[i] * strides[i];
        return result;
    }
    public float load(int idx[D])
    {
        return data[index(idx)];
    }
    public void store(int idx[D], float value)
    {
        data[index(idx)] = value;
    }
    public void add(int idx[D], float value)
    {
        float old;
        InterlockedAdd(data[index(idx)], value, old);
    }
    public void copy(out This value)
    {
        value = this;
    }
}
public struct DiffTensor<let D : int>
{
    public Tensor<D> primal;
    public Tensor<D> gradient;
    [ForceInline]
    public void loadBackward(int idx[D], float gradient)
    {
        this.gradient.add(idx, gradient);
    }
    [ForceInline]
    [Differentiable]
    [BackwardDerivative(loadBackward)]
    public float load(int idx[D])
    {
        return primal.load(idx);
    }
    [Differentiable]
    public void loadInto(Context<D> context, out float value)
    {
        value = load(context.callID);
    }
}
public struct WDiffTensor<let D : int>
{
    public Tensor<D> primal;
    public Tensor<D> gradient;
    [ForceInline]
    public void storeBackward(int idx[D], inout DifferentialPair<float> value)
    {
        value = diffPair(value.p, gradient.load(idx));
    }
    [ForceInline]
    [Differentiable]
    [BackwardDerivative(storeBackward)]
    public void store(int idx[D], float value)
    {
        primal.store(idx, value);
    }
    [Differentiable]
    public void storeValue(Context<D> context, float value)
    {
        store(context.callID, value);
    }
}

[Differentiable]
void polynomial(float a, float b, float c, float x, out float result)
{
    result = a * x * x + b * x + c;
}
static const int mapping[1] = {0};
[Differentiable]
void trampoline(
    Context<1> context,
    no_diff float a,
    no_diff float b,
    no_diff float c,
    no_diff DiffTensor<1> x,
    no_diff WDiffTensor<1> y)
{
    float input;
    float result;
    x.loadInto(context.map(mapping), input);
    polynomial(a, b, c, input, result);
    y.storeValue(context.map(mapping), result);
}
[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid : SV_DispatchThreadID,
    uniform DiffTensor<1> x,
    uniform WDiffTensor<1> y,
    uniform float a,
    uniform float b,
    uniform float c)
{
    Context<1> context = {tid, {int(tid.x)}};
    bwd_diff(trampoline)(context, a, b, c, x, y);
}
