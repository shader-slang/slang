// Test the pattern from issue #9884: using explicit IProcessor type annotation
// instead of `let`. This was broken for associated type scope tracking prior to
// #10004 — the compiler created separate temporaries for each access, causing
// the associated type from proc.Handler to differ between uses.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IHandler
{
    int handle(int x);
}

struct DoubleHandler : IHandler
{
    int handle(int x) { return x * 2; }
}

struct TripleHandler : IHandler
{
    int handle(int x) { return x * 3; }
}

interface IProcessor
{
    associatedtype Handler : IHandler;
    Handler getHandler();
}

struct ProcessorA : IProcessor
{
    typealias Handler = DoubleHandler;
    DoubleHandler getHandler() { return DoubleHandler(); }
}

struct ProcessorB : IProcessor
{
    typealias Handler = TripleHandler;
    TripleHandler getHandler() { return TripleHandler(); }
}

IProcessor createProcessor(int id)
{
    if (id == 0)
        return ProcessorA();
    else
        return ProcessorB();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Using explicit IProcessor annotation with associated type — fixed in #10004
    IProcessor proc = createProcessor(0);
    proc.Handler h = proc.getHandler();
    outputBuffer[0] = h.handle(5);  // CHECK: 10
}
