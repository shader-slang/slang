// Checks the values that device float atomic load, exchange and compare-exchange return and
// store on Metal. In each pair of compare-exchanges the first one matches and stores, and the
// second one does not. The CUDA run checks the same values on machines without Metal.

//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[1.0], stride=4):name=atomicBuffer
RWStructuredBuffer<Atomic<float>> atomicBuffer;
//TEST_INPUT:ubuffer(data=[2.0], stride=4):name=floatBuffer
RWStructuredBuffer<float> floatBuffer;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = atomicBuffer[0].load();
    outputBuffer[1] = atomicBuffer[0].exchange(3.0f);
    outputBuffer[2] = atomicBuffer[0].compareExchange(3.0f, 4.0f);
    outputBuffer[3] = atomicBuffer[0].compareExchange(3.0f, 5.0f);
    outputBuffer[4] = atomicBuffer[0].load();

    float original;
    InterlockedExchange(floatBuffer[0], 6.0f, original);
    outputBuffer[5] = original;
    InterlockedCompareExchange(floatBuffer[0], 6.0f, 7.0f, original);
    outputBuffer[6] = original;
    InterlockedCompareExchange(floatBuffer[0], 6.0f, 8.0f, original);
    outputBuffer[7] = original;
    outputBuffer[8] = floatBuffer[0];
}

// BUF: type: float
// BUF-NEXT: 1.0
// BUF-NEXT: 1.0
// BUF-NEXT: 3.0
// BUF-NEXT: 4.0
// BUF-NEXT: 4.0
// BUF-NEXT: 2.0
// BUF-NEXT: 6.0
// BUF-NEXT: 7.0
// BUF-NEXT: 7.0
