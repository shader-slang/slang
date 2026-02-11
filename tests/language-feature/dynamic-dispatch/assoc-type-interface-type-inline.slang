// Test inline usage of associated type that conforms to an interface,
// without intermediate variables: proc.getHandler().handle(x).
// Also tests explicit IHandler annotation on the returned handler.
// Issue #9884
//
// Uses createDynamicObject with type_conformance to ensure runtime dispatch
// is not specialized away by the compiler.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

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
    float pad;  // Data field for createDynamicObject AnyValue packing
    DoubleHandler getHandler() { return DoubleHandler(); }
}

struct ProcessorB : IProcessor
{
    typealias Handler = TripleHandler;
    float pad;  // Data field for createDynamicObject AnyValue packing
    TripleHandler getHandler() { return TripleHandler(); }
}

//TEST_INPUT: type_conformance ProcessorA:IProcessor = 0
//TEST_INPUT: type_conformance ProcessorB:IProcessor = 1

IProcessor createProcessor(uint id, float data)
{
    return createDynamicObject<IProcessor>(id, data);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test 1: Inline chained call (no intermediate variable for handler)
    let proc1 = createProcessor(0, 1.0);
    outputBuffer[0] = proc1.getHandler().handle(5);
    // CHECK: 10

    // Test 2: Inline chained call with ProcessorB
    let proc2 = createProcessor(1, 2.0);
    outputBuffer[1] = proc2.getHandler().handle(5);
    // CHECK: 15

    // Test 3: Explicit IHandler annotation on returned handler
    let proc3 = createProcessor(0, 3.0);
    IHandler h3 = proc3.getHandler();
    outputBuffer[2] = h3.handle(7);
    // CHECK: 14

    // Test 4: Explicit IHandler annotation with ProcessorB
    let proc4 = createProcessor(1, 4.0);
    IHandler h4 = proc4.getHandler();
    outputBuffer[3] = h4.handle(7);
    // CHECK: 21
}
