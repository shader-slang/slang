//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cpu -entry computeMain -stage compute

//
// Limitation: Explicit Type Annotation
//
// Using `IContainer obj = ...` instead of `let obj = ...` breaks associated type tracking.
// Both declarations should be semantically equivalent, but the explicit annotation causes
// the compiler to lose the connection between obj.Element and the methods on obj.
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

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    // Works: let with type inference
    {
        let obj = makeContainer(0);
        obj.Element x = obj.get();
        outputBuffer[0] = obj.toFloat();  // OK
    }

    // Broken: explicit IContainer annotation
    // CHECK: error 30019
    {
        IContainer obj = makeContainer(0);
        obj.Element x = obj.get();
        outputBuffer[1] = obj.toFloat();
    }
}
