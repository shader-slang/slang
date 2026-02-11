//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

//
// Limitation: Mutating Methods
//
// Using [mutating] methods with associated type parameters fails.
// Reading with obj.get() and writing with obj.set(x) on the same object should work.
// The value x has type obj.Element, and set expects obj.Element — they're the same type
// from the same object. The [mutating] attribute shouldn't break associated type tracking.
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

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    var obj = makeContainer(0);
    obj.Element x = obj.get();

    // Cannot pass associated type value to [mutating] method on the same object
    // CHECK: error 30019
    obj.set(x);

    outputBuffer[0] = obj.toFloat();
}
