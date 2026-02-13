//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites -conformance "FooA:IFoo=0" -conformance "FooB:IFoo=1" -conformance "BarA:IBar=0" -conformance "BarB:IBar=1" -conformance "MulCombiner:ICombiner=0" -conformance "AddCombiner:ICombiner=1"

// Test for GitHub issue #9838: Dynamic Dispatch with Dynamic Dispatch Parameters
// Interface methods that take other dynamically-typed interface parameters should work correctly.
// This tests nested dynamic dispatch where ICombiner.combine takes IFoo and IBar as existential params.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float eval();
}

interface IBar
{
    float weight();
}

interface ICombiner
{
    float combine(IFoo foo, IBar bar); // Both params are existential
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

struct BarA : IBar
{
    float val;
    float weight() { return val; }
}

struct BarB : IBar
{
    float val;
    float weight() { return val * 0.5; }
}

struct MulCombiner : ICombiner
{
    float dummy;  // Data field for createDynamicObject
    
    float combine(IFoo foo, IBar bar)
    {
        // Nested dynamic dispatch: foo.eval() and bar.weight()
        return foo.eval() * bar.weight();
        // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
        // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

struct AddCombiner : ICombiner
{
    float dummy;  // Data field for createDynamicObject
    
    float combine(IFoo foo, IBar bar)
    {
        // Different combiner implementation
        return foo.eval() + bar.weight();
        // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
        // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

// Create interface values with runtime type IDs to prevent compile-time specialization
IFoo createFoo(uint id, float data)
{
    return createDynamicObject<IFoo>(id, data);
}

IBar createBar(uint id, float data)
{
    return createDynamicObject<IBar>(id, data);
}

ICombiner createCombiner(uint id, float dummy)
{
    return createDynamicObject<ICombiner>(id, dummy);
}

float testCombine(uint combinerId, uint fooId, float fooData, uint barId, float barData)
{
    ICombiner c = createCombiner(combinerId, 0.0);
    IFoo f = createFoo(fooId, fooData);
    IBar b = createBar(barId, barData);
    
    // Triple dynamic dispatch happens here:
    // 1. Dispatch to correct ICombiner implementation
    // 2. Inside combine(), dispatch foo.eval()
    // 3. Inside combine(), dispatch bar.weight()
    return c.combine(f, b);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

//TEST_INPUT: type_conformance FooA:IFoo = 0
//TEST_INPUT: type_conformance FooB:IFoo = 1
//TEST_INPUT: type_conformance BarA:IBar = 0
//TEST_INPUT: type_conformance BarB:IBar = 1
//TEST_INPUT: type_conformance MulCombiner:ICombiner = 0
//TEST_INPUT: type_conformance AddCombiner:ICombiner = 1

[numthreads(1, 1, 1)]
void computeMain()
{
    // Test 1: MulCombiner(0) with FooA(0, 1.0) * BarA(0, 0.5) = 1.0 * 0.5 = 0.5
    outputBuffer[0] = testCombine(0, 0, 1.0, 0, 0.5);
    // CHECK: 0.5

    // Test 2: MulCombiner(0) with FooB(1, 1.0) * BarB(1, 1.0) = 2.0 * 0.5 = 1.0
    outputBuffer[1] = testCombine(0, 1, 1.0, 1, 1.0);
    // CHECK: 1.0

    // Test 3: AddCombiner(1) with FooA(0, 1.0) + BarA(0, 0.5) = 1.0 + 0.5 = 1.5
    outputBuffer[2] = testCombine(1, 0, 1.0, 0, 0.5);
    // CHECK: 1.5

    // Test 4: AddCombiner(1) with FooB(1, 1.0) + BarB(1, 1.0) = 2.0 + 0.5 = 2.5
    outputBuffer[3] = testCombine(1, 1, 1.0, 1, 1.0);
    // CHECK: 2.5
}
