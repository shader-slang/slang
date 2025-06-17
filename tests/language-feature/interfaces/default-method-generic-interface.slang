//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type

// Test that a generic interface method in a generic interface
// can have a body providing default implementation.

interface IFoo<int v>
{
    int getVal();
    int getGreaterVal<int x>()
    {
        return getVal() + x + v;
    }
}

struct Impl : IFoo<2>
{
    int getVal()
    {
        return 42;
    }
    
    // Using the default implementation for getGreaterVal.
}

struct Impl2 : IFoo<2>
{
    int getVal()
    {
        return 42;
    }
    
    // overriding default implementation.
    override int getGreaterVal<int x>()
    {
        return 100 + x;
    }
}

int test<int y, T:IFoo<y>>(T v) { return v.getGreaterVal<1>(); }

//TEST_INPUT: set resultBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> resultBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Impl impl = {};
    int result = test(impl);
    resultBuffer[0] = result;
    // CHECK: 45

    Impl2 impl2 = {};
    resultBuffer[1] = test(impl2);
    // CHECK: 101
}