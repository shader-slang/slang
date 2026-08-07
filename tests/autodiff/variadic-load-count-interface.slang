//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[3.0 0.0 0.0 0.0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface ITensor<T : IDifferentiable, let N : int> : IDifferentiable
{
    [Differentiable]
    T load<each TIndex>(TIndex indices)
        where TIndex == int
        where countof(TIndex) == N;
}

struct OutputTensor : IDifferentiable, ITensor<float, 2>
{
    typealias Differential = OutputTensor;

    float scale;

    [Differentiable]
    float load<each TIndex>(TIndex indices)
        where TIndex == int
        where countof(TIndex) == 2
    {
        return scale * outputBuffer[0] + float(countof(TIndex));
    }

    [Differentiable]
    static Differential dzero()
    {
        Differential result;
        result.scale = 0.0;
        return result;
    }

    [Differentiable]
    static Differential dadd(Differential left, Differential right)
    {
        Differential result;
        result.scale = left.scale + right.scale;
        return result;
    }
}

[Differentiable]
float load2D<TTensor : ITensor<float, 2>>(TTensor tensor)
{
    return tensor.load(1, 2);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    OutputTensor tensor;
    tensor.scale = 2.0;

    OutputTensor tangent = OutputTensor.dzero();
    tangent.scale = 0.5;

    let result = fwd_diff(load2D<OutputTensor>)(diffPair(tensor, tangent));

    outputBuffer[1] = result.p;
    outputBuffer[2] = result.d;

    // CHECK: 8.0
    // CHECK: 1.5
}
