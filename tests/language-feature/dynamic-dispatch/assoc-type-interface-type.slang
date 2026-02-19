// Associated type constrained to conform to an interface creates nested
// dynamic dispatch: first dispatch resolves the IProcessor method, then
// the returned Handler (which conforms to IHandler) requires a second dispatch.
// Issue #9884
//
// Uses createDynamicObject with type_conformance to ensure runtime dispatch
// is not specialized away by the compiler.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites -conformance "ProcessorA:IProcessor=0" -conformance "ProcessorB:IProcessor=1"

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
    // Test 1: ProcessorA -> DoubleHandler -> handle(5) = 10
    let procA = createProcessor(0, 1.0);
    let hA = procA.getHandler();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[0] = hA.handle(5);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 10

    // Test 2: ProcessorB -> TripleHandler -> handle(5) = 15
    let procB = createProcessor(1, 2.0);
    let hB = procB.getHandler();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[1] = hB.handle(5);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 15

    // Test 3: Using associated type annotation (proc.Handler)
    let proc2 = createProcessor(0, 3.0);
    proc2.Handler h2 = proc2.getHandler();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[2] = h2.handle(3);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 6

    // Test 4: Associated type annotation with ProcessorB
    let proc3 = createProcessor(1, 4.0);
    proc3.Handler h3 = proc3.getHandler();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[3] = h3.handle(4);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // CHECK: 12
}
