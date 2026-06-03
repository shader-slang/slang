//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// Test: Explicit Type Annotation with Associated Types
//
// Using `IContainer obj = ...` should behave the same as `let obj = ...` for
// associated type tracking. Both declarations bind the same existential value,
// so obj.Element should resolve consistently.
//
// This was broken prior to #10004: the compiler created separate temporary
// variables for each access to a non-let existential, causing the associated
// type from obj.Element to differ from the return type of obj.get().
//

interface IContainer
{
    associatedtype Element;
    Element get();
    float toFloat();
}

struct IntContainer : IContainer
{
    typealias Element = int;
    int val;

    __init(int v) { val = v; }

    int get() { return val; }
    float toFloat() { return float(val); }
}

struct FloatContainer : IContainer
{
    typealias Element = float;
    float val;

    __init(float v) { val = v; }

    float get() { return val; }
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
    // Test 1: let with type inference — always worked
    {
        let obj = makeContainer(0);
        obj.Element x = obj.get();
        outputBuffer[0] = obj.toFloat();  // CHECK: 42.0
    }

    // Test 2: explicit IContainer annotation — fixed in #10004
    {
        IContainer obj = makeContainer(1);
        obj.Element x = obj.get();
        outputBuffer[1] = obj.toFloat();  // CHECK: 3.5
    }
}
