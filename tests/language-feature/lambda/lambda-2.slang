
//TEST:COMPARE_COMPUTE(targets=vk+cuda+cpu, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

IFunc<float, float> createFunc(int c)
{
    return (float x) => (float)(x * 2 + c);
}


[numthreads(1, 1, 1)]
void computeMain()
{
    let func = createFunc(2);
    // CHECK: 8.0
    outputBuffer[0] = func(3.0);
}
