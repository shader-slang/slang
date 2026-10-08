// Regression test for https://github.com/shader-slang/slang/issues/12923
// A lambda inside an interface default method may capture `this`: the receiver is
// registered as a closure capture, and the synthesized closure field is named so it
// does not collide with the reserved `This` name during constructor synthesis. Both
// explicit `this.getVal()` and unqualified `getVal()` lookup must reach the captured
// receiver in the lambda body.

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

    int computeThroughUnqualifiedLookup()
    {
        let f = () => getVal() * 3;
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

int testUnqualifiedLookup<T : IFoo>(T v)
{
    return v.computeThroughUnqualifiedLookup();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Impl impl = {};
    outputBuffer[0] = test(impl);
    // CHECK: 42
    outputBuffer[1] = testUnqualifiedLookup(impl);
    // CHECK: 63
}
