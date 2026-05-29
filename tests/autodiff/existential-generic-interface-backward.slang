// Test backward differentiation through a generic interface with [BackwardDerivative].
// Regression test for https://github.com/shader-slang/slangpy/issues/639

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

interface IData<T, let D : int>
{
    T read(int idx);
}

struct Data<T, let D : int> : IData<T, D>
{
    StructuredBuffer<T> buffer;

    T read(int idx) { return buffer[idx]; }
}

//TEST_INPUT: set inputData = ubuffer(data=[1.0 2.0 3.0 4.0], stride=4)
StructuredBuffer<float> inputData;

[Differentiable]
float process(int idx, IData<float, 1> data)
{
    return data.read(idx);
}

[BackwardDerivative(compute_bwd)]
float compute_fn(int idx, IData<float, 1> data)
{
    return process(idx, data);
}

void compute_bwd(int idx, IData<float, 1> data, float df)
{
    bwd_diff(process)(idx, data, df);
}

[Differentiable]
float trampoline(float scale, no_diff int idx, no_diff Data<float, 1> data)
{
    return scale * compute_fn(idx, data);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Data<float, 1> data;
    data.buffer = inputData;

    outputBuffer[0] = trampoline(2.0, 0, data);
    // CHECK: 2.0

    outputBuffer[1] = trampoline(3.0, 1, data);
    // CHECK: 6.0

    {
        var dp = diffPair(2.0);
        bwd_diff(trampoline)(dp, 0, data, 1.0);
        outputBuffer[2] = dp.d;
    }
    // CHECK: 1.0

    {
        var dp = diffPair(3.0);
        bwd_diff(trampoline)(dp, 1, data, 1.0);
        outputBuffer[3] = dp.d;
    }
    // CHECK: 2.0
}
