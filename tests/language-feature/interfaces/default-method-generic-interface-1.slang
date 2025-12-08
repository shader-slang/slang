//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type

// Test that a generic interface method in a generic interface
// can have a body providing default implementation.
// In addition, the implementation type is also a generic.
// See: https://github.com/shader-slang/slang/issues/9202

interface IFoo<int v>
{
    int getVal();
    int getGreaterVal<int x>()
    {
        return getVal() + x + v;
    }
}

struct Impl<int v> : IFoo<v>
{
    int getVal()
    {
        return 42;
    }
    // Using the default implementation for getGreaterVal.
}

int test<int y, T:IFoo<y>>(T v) { return v.getGreaterVal<1>(); }

//TEST_INPUT: set resultBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> resultBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Impl<2> impl = {};
    int result = test(impl);
    resultBuffer[0] = result;
    // CHECK: 45
}
