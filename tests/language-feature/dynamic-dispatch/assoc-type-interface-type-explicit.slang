// Test the exact pattern from issue #9884: using explicit IProcessor
// type annotation instead of `let`. This is known to be fragile for
// associated type scope tracking (see issue #9883 explicit annotation gap).
//
// Uses createDynamicObject with type_conformance to ensure runtime dispatch
// is not specialized away by the compiler.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

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
    float pad;
    DoubleHandler getHandler() { return DoubleHandler(); }
}

struct ProcessorB : IProcessor
{
    typealias Handler = TripleHandler;
    float pad;
    TripleHandler getHandler() { return TripleHandler(); }
}

IProcessor createProcessor(uint id, float data)
{
    return createDynamicObject<IProcessor>(id, data);
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Using explicit IProcessor annotation breaks associated type
    // scope tracking — the compiler treats proc.Handler at different
    // scope depths, causing a false type mismatch.
    // CHECK: error 30019
    IProcessor proc = createProcessor(0, 1.0);
    proc.Handler h = proc.getHandler();
    outputBuffer[0] = h.handle(5);
}
