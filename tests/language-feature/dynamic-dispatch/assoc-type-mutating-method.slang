//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// Test: Mutating Methods with Associated Type Parameters
//
// Reading with obj.get() and writing with obj.set(x) on the same object should
// work. The value x has type obj.Element, and set expects obj.Element — they're
// the same type from the same object.
//
// This was broken prior to #10004: the compiler created separate temporary
// variables for each access to a mutable existential, causing the associated
// type scope to differ between get() and set() calls.
//

interface IContainer
{
    associatedtype Element;
    Element get();
    [mutating] void set(Element e);
    float toFloat();
}

struct IntContainer : IContainer
{
    typealias Element = int;
    int val;

    __init(int v) { val = v; }

    int get() { return val; }
    [mutating] void set(int e) { val = e; }
    float toFloat() { return float(val); }
}

struct FloatContainer : IContainer
{
    typealias Element = float;
    float val;

    __init(float v) { val = v; }

    float get() { return val; }
    [mutating] void set(float e) { val = e; }
    float toFloat() { return val; }
}

IContainer makeContainer(int id)
{
    if (id == 0)
        return IntContainer(42);
    else
        return FloatContainer(3.5);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    // Test 1: get + set on the same object — fixed in #10004
    {
        var obj = makeContainer(0);
        obj.Element x = obj.get();
        obj.set(x);
        outputBuffer[0] = obj.toFloat();  // CHECK: 42.0
    }

    // Test 2: set with a different value — fixed in #10004
    {
        var obj = makeContainer(1);
        obj.Element x = obj.get();
        obj.set(x);
        outputBuffer[1] = obj.toFloat();  // CHECK: 3.5
    }
}
