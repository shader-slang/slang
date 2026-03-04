//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type

// Test that a generic interface method can have a body providing default implementation.

interface IFoo
{
    int getVal();
    int getGreaterVal<int x>()
    {
        return getVal() + x;
    }
}

struct Impl : IFoo
{
    int getVal()
    {
        return 42;
    }
    
    // override default implementation for getGreaterVal.
    override int getGreaterVal<int x>()
    {
        return getVal() + x + 1; // Adding 1 to differentiate from the default implementation.
    }
}

struct Other
{
    int n;
}

extension Other: IFoo
{
    int getVal()
    {
        return n;
    }

    // Override should also work inside an extension
    override int getGreaterVal<int x>()
    {
        return getVal() + x + 2;
    }
}

int test<T:IFoo>(T v) { return v.getGreaterVal<1>(); }

//TEST_INPUT: set resultBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> resultBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Impl impl = {};
    int result = test(impl);
    resultBuffer[0] = result;
    // CHECK: 44

    Other other = {32};
    resultBuffer[1] = test(other);
    // CHECK-NEXT: 35
}
