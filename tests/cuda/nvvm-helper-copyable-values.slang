//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[2 3 4], stride=4):name=inputBuffer
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
StructuredBuffer<int> inputBuffer;
RWStructuredBuffer<uint> outputBuffer;

struct Payload
{
    double values[2];
    int bias;
};

[noinline]
double evaluate(Payload value, out double sum)
{
    sum = value.values[0] + value.values[1];
    return sum + double(value.bias);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Payload value;
    value.values[0] = double(inputBuffer[0]);
    value.values[1] = double(inputBuffer[1]);
    value.bias = inputBuffer[2];

    double sum;
    double result = evaluate(value, sum);
    outputBuffer[0] = uint(result + sum);

    // CHECK: 14
}
