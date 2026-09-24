// Test case for GitHub issue #8148
// Crash when using static consts in generic interface methods with where clauses

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -shaderobj -output-using-type

interface IVector<T, int N>
{
    int getValue();
}

struct MyVector<T, int N> : IVector<T, N>
{
    int getValue()
    {
        return N;
    }
}

interface IEncoder<T>
{
    static const int InDim;
    static const int OutDim;

    OutputVector eval<InputVector, OutputVector>(InputVector input)
        where InputVector : IVector<T, InDim>
        where OutputVector : IVector<T, OutDim>;
}

struct Encoder<T> : IEncoder<T>
{
    static const int InDim = 4;
    static const int OutDim = 2;

    OutputVector eval<InputVector, OutputVector>(InputVector input)
        where InputVector : IVector<T, InDim>
        where OutputVector : IVector<T, OutDim>
    {
        OutputVector ret;
        return ret;
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    Encoder<float> encoder;
    MyVector<float, 4> input;
    MyVector<float, 2> output = encoder.eval<MyVector<float, 4>, MyVector<float,2>>(input);
    outputBuffer[0] = input.getValue() + output.getValue();
    // CHK: 6
}
