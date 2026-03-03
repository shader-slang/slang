//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// Test: Accessing an associated type through a dynamically dispatched interface value.
// The compiler should resolve obj.Element correctly even when obj's concrete type
// is determined at runtime.
//
// Scenarios tested:
//   1. Reading via associated type: obj.Element x = obj.get()
//   2. Passing associated type value to non-mutating method: obj.process(x)
//   3. Multiple concrete implementations with different Element types
//
// Known limitation: passing associated type value to [mutating] method obj.set(x)
// fails with type mismatch error 30019. See assoc-type-mutating-method.slang.
//

interface IContainer
{
    associatedtype Element;
    Element get();
    [mutating] void set(Element e);
    float process(Element e);  // Non-mutating: accept associated type and return float
    float toFloat();           // Convert stored value to float for test verification
}

struct IntContainer : IContainer
{
    typealias Element = int;
    int val;

    __init(int v) { val = v; }

    int get() { return val; }
    [mutating] void set(int e) { val = e; }
    float process(int e) { return float(e) + 0.5; }
    float toFloat() { return float(val); }
}

struct FloatContainer : IContainer
{
    typealias Element = float;
    float val;

    __init(float v) { val = v; }

    float get() { return val; }
    [mutating] void set(float e) { val = e; }
    float process(float e) { return e * 2.0; }
    float toFloat() { return val; }
}

IContainer makeContainer(int id)
{
    if (id == 0)
        return IntContainer(42);
    else
        return FloatContainer(3.5);
}

// Test read via associated type
float testRead(int id)
{
    let obj = makeContainer(id);
    obj.Element x = obj.get();  // Read via associated type
    return obj.toFloat();
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

// Test passing associated type value to non-mutating method
float testProcess(int id)
{
    let obj = makeContainer(id);
    obj.Element x = obj.get();     // Read via associated type
    return obj.process(x);         // Pass associated type value to non-mutating method
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    // Test 1: IntContainer - read via associated type
    outputBuffer[0] = testRead(0);  // CHECK: 42.0

    // Test 2: FloatContainer - read via associated type
    outputBuffer[1] = testRead(1);  // CHECK: 3.5

    // Test 3: IntContainer - pass associated type to non-mutating method
    // IntContainer.process(42) = float(42) + 0.5 = 42.5
    outputBuffer[2] = testProcess(0);  // CHECK: 42.5

    // Test 4: FloatContainer - pass associated type to non-mutating method
    // FloatContainer.process(3.5) = 3.5 * 2.0 = 7.0
    outputBuffer[3] = testProcess(1);  // CHECK: 7.0

    // Test 5: Inline read pattern - IntContainer
    {
        let c = makeContainer(0);
        c.Element val = c.get();
        outputBuffer[4] = c.toFloat();  // CHECK: 42.0
    }

    // Test 6: Inline read pattern - FloatContainer
    {
        let c = makeContainer(1);
        c.Element val = c.get();
        outputBuffer[5] = c.toFloat();  // CHECK: 3.5
    }

    // Test 7: Inline process pattern - IntContainer
    {
        let c = makeContainer(0);
        c.Element val = c.get();
        outputBuffer[6] = c.process(val);  // CHECK: 42.5
    }

    // Test 8: Inline process pattern - FloatContainer
    {
        let c = makeContainer(1);
        c.Element val = c.get();
        outputBuffer[7] = c.process(val);  // CHECK: 7.0
    }
}
