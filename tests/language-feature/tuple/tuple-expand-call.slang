//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)

#lang 2026

RWStructuredBuffer<int> outputBuffer;

int f(int x, float y) { return x + int(y); }

[numthreads(1,1,1)]
void computeMain()
{
    Tuple<expand each Tuple<int, float>> x = (2, 3.0f);
    // CHECK: 5
    outputBuffer[0] = f(expand each x);
}
