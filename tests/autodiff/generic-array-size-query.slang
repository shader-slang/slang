//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:COMPARE_COMPUTE(targets=all,filecheck-buffer=CHECK):-output-using-type
//TEST_INPUT:ubuffer(data=[0],stride=4):out,name=outputBuffer

struct SizedArray<T : IDifferentiable, let N : int> : IDifferentiable
{
    static const int Alignment = sizeof(uint4) / sizeof(half);
    static const int AlignedSize = ((N + Alignment - 1) / Alignment) * Alignment;
    T[AlignedSize] values;

    [Differentiable]
    [mutating]
    void set(int index, T value)
    {
        // The folded size and its canonical value identify the same differentiable array type.
        values[index] = value;
    }
}

[Differentiable]
float compute(float x)
{
    SizedArray<float, 3> data = {};
    data.set(0, x);
    return data.values[0];
}

RWStructuredBuffer<float> outputBuffer;

// HLSL: void computeMain
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = fwd_diff(compute)(diffPair(2.0, 1.0)).d; // CHECK: 1.0
}
