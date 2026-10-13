// Regression test for https://github.com/shader-slang/slang/issues/12923
// A lambda inside an interface default method may capture `this`: semantic checking creates a
// provisional closure field named `$this`, and selected-expression finalization attaches it only
// for the selected capture path. The distinct name avoids colliding with reserved `This` during
// constructor synthesis. Both an explicit `this.getVal()` and an unqualified `getVal()` inside the
// lambda must use the captured receiver.

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
