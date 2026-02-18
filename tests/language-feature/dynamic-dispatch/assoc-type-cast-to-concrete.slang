//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cpu -entry computeMain -stage compute

//
// Limitation: Direct Cast to Concrete Type
//
// Cannot cast associated type value to a concrete type directly.
// If Element is constrained to types that can convert to float (or if the concrete
// type is int), then float(x) should work. The compiler should be able to emit a
// dynamic dispatch that performs the appropriate conversion based on the runtime type.
//

interface IContainer
{
    associatedtype Element;
    Element get();
}

struct IntContainer : IContainer
{
    typealias Element = int;
    int val;

    __init(int v) { val = v; }

    int get() { return val; }
}

struct FloatContainer : IContainer
{
    typealias Element = float;
    float val;

    __init(float v) { val = v; }

    float get() { return val; }
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
    let obj = makeContainer(0);
    obj.Element x = obj.get();

    // Cannot cast associated type value to float directly
    // CHECK: ([[# @LINE+1]]): error 30019
    outputBuffer[0] = float(x);
}
