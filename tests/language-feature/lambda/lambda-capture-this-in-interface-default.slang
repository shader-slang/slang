// Regression test for https://github.com/shader-slang/slang/issues/12923
// A lambda inside an interface default method may capture `this`: the receiver is
// registered as a closure capture, and the synthesized closure field is named so it
// does not collide with the reserved `This` name during constructor synthesis. The
// captured `this` must reach the lambda body, so `f()` returns getVal() * 2 == 42.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

interface IFoo
{
    int getVal();
    int compute()
    {
        let f = () => this.getVal() * 2;
        return f();
    }
}

struct Impl : IFoo
{
    int getVal() { return 21; }
}

int test<T : IFoo>(T v)
{
    return v.compute();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Impl impl = {};
    outputBuffer[0] = test(impl);
    // CHECK: 42
}
