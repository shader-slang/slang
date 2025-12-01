//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
struct Ty<T>
{
    static int x[2];
    int doSomething()
    {
        x[0] = 42;
        return x[0];
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride = 4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 42
    Ty<int> x;
    outputBuffer[0] = x.doSomething();
}
