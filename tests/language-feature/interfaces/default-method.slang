//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type

// Test that interface method can have a body providing default implementation.

interface IFoo
{
    int getVal();
    int getGreaterVal()
    {
        return getVal() + 1;
    }
    static int getStaticVal()
    {
        return 100;
    }
}

struct Impl : IFoo
{
    int getVal()
    {
        return 42;
    }
    
    // Using the default implementation for getGreaterVal.
}

int test<T:IFoo>(T v) { return v.getGreaterVal(); }

int test2<T:IFoo>() { return T.getStaticVal();}

//TEST_INPUT: set resultBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> resultBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Impl impl = {};
    int result = test(impl);
    resultBuffer[0] = result;
    // CHECK: 43

    resultBuffer[1] = test2<Impl>();
    // CHECK: 100
}