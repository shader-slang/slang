//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cpu -entry computeMain -stage compute

//
// Limitation: Cross-Object Data Passing
//
// Cannot pass associated type data between two interface variants, even when
// they're the same concrete type. When obj1 and obj2 are created from the same
// factory and have the same concrete type, their associated types are identical
// at runtime. The compiler treats obj1.Data and obj2.Data as different types,
// but they resolve to the same concrete type and should be interchangeable.
//

interface IProcessor
{
    associatedtype Data;
    Data make();
    float process(Data d);
}

struct FloatProcessor : IProcessor
{
    typealias Data = float;
    float val;

    __init(float v) { val = v; }

    float make() { return val; }
    float process(float d) { return d * 2.0; }
}

struct IntProcessor : IProcessor
{
    typealias Data = int;
    int val;

    __init(int v) { val = v; }

    int make() { return val; }
    float process(int d) { return float(d) + 0.5; }
}

IProcessor makeProcessor(int id)
{
    if (id == 0)
        return FloatProcessor(3.5);
    else
        return IntProcessor(42);
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    let obj1 = makeProcessor(0);
    let obj2 = makeProcessor(0);  // Same concrete type as obj1

    obj1.Data d = obj1.make();

    // Cannot pass obj1.Data to obj2.process(), even though both are the same type
    // CHECK: ([[# @LINE+1]]): error 30019
    outputBuffer[0] = obj2.process(d);
}
