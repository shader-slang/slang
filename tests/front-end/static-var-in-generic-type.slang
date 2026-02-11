//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
// Disabled on LLVM due to lack of barrier support
//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -llvm
struct Ty<T>
{
    static int x[2];
    static groupshared T y[2];

    T gDoSomething(T v)
    {
        y[0] = v;
        return y[0];
    }
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

    x.gDoSomething(5);
    Ty<float> y;
    y.gDoSomething(3.0);

    GroupMemoryBarrierWithGroupSync();

    // CHECK: 8
    outputBuffer[1] = (int)Ty<float>.y[0] + x.y[0];

}
