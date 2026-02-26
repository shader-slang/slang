// Dynamically dispatched methods returning interface-typed values.
// Wrapper functions must correctly handle packing for:
// 1. Direct interface return values (tag + AnyValue)
// 2. Out parameters writing back complete existential representations
// 3. Inout parameters preserving the ability to change concrete type
//
// For composite return containing interfaces (Tuple<IFoo, int>), see
// return-interface-tuple-limitation.slang (currently crashes the compiler).

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites -conformance "FactoryA:IFactory=0" -conformance "FactoryB:IFactory=1" -conformance "FooA:IFoo=0" -conformance "FooB:IFoo=1"

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT: type_conformance FactoryA:IFactory = 0
//TEST_INPUT: type_conformance FactoryB:IFactory = 1
//TEST_INPUT: type_conformance FooA:IFoo = 0
//TEST_INPUT: type_conformance FooB:IFoo = 1

interface IFoo
{
    float eval();
}

interface IFactory
{
    IFoo create();
    void createOut(out IFoo result);
    void modify(inout IFoo item);
}

struct FooA : IFoo
{
    float val;
    float eval() { return val; }
}

struct FooB : IFoo
{
    float val;
    float eval() { return val * 2.0; }
}

struct FactoryA : IFactory
{
    float dummy;
    IFoo create() { return FooA(3.0); }
    void createOut(out IFoo result) { result = FooA(7.0); }
    void modify(inout IFoo item) { item = FooB(11.0); }
}

struct FactoryB : IFactory
{
    float dummy;
    IFoo create() { return FooB(4.0); }
    void createOut(out IFoo result) { result = FooB(8.0); }
    void modify(inout IFoo item) { item = FooA(13.0); }
}

IFactory createFactory(uint id)
{
    return createDynamicObject<IFactory>(id, 0.0);
}

float testCreate(int id)
{
    IFactory factory = createFactory(uint(id));
    IFoo foo = factory.create();
    return foo.eval();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float testCreateOut(int id)
{
    IFactory factory = createFactory(uint(id));
    IFoo result;
    factory.createOut(result);
    return result.eval();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float testModify(int id)
{
    IFactory factory = createFactory(uint(id));
    IFoo item = FooA(100.0);
    factory.modify(item);
    return item.eval();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // Pattern 1: Direct interface return
    // FactoryA.create() -> FooA(3.0).eval() = 3.0
    outputBuffer[0] = testCreate(id);
    // CHECK: 3.0
    // FactoryB.create() -> FooB(4.0).eval() = 4.0 * 2.0 = 8.0
    outputBuffer[1] = testCreate(id + 1);
    // CHECK: 8.0

    // Pattern 2: Out parameter
    // FactoryA.createOut -> FooA(7.0).eval() = 7.0
    outputBuffer[2] = testCreateOut(id);
    // CHECK: 7.0
    // FactoryB.createOut -> FooB(8.0).eval() = 8.0 * 2.0 = 16.0
    outputBuffer[3] = testCreateOut(id + 1);
    // CHECK: 16.0

    // Pattern 3: Inout parameter (concrete type changes)
    // FactoryA.modify(FooA(100)) -> replaces with FooB(11.0) -> 11.0 * 2.0 = 22.0
    outputBuffer[4] = testModify(id);
    // CHECK: 22.0
    // FactoryB.modify(FooA(100)) -> replaces with FooA(13.0) -> 13.0
    outputBuffer[5] = testModify(id + 1);
    // CHECK: 13.0
}
