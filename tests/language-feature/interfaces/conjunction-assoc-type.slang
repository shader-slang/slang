// conjunction-assoc-type.slang

// Test that `&` works in associated type constraints.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IReadable
{
    int read();
}

interface IWritable
{
    [mutating] void write(int v);
}

// Interface with an associated type constrained by a conjunction.
interface IContainer
{
    associatedtype Element : IReadable & IWritable;

    Element getElement();
}

struct MyElement : IReadable & IWritable
{
    int val;

    int read() { return val; }

    [mutating] void write(int v) { val = v; }
}

struct MyContainer : IContainer
{
    typedef MyElement Element;
    int stored;

    MyElement getElement()
    {
        MyElement e;
        e.val = stored;
        return e;
    }
}

int transformElement<C : IContainer>(C container)
{
    // The associated type Element should satisfy both IReadable and IWritable.
    C.Element elem = container.getElement();
    int original = elem.read();
    elem.write(original * 3);
    return elem.read();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// CHECK: 0
// CHECK: 3
// CHECK: 6
// CHECK: 9

[numthreads(4, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    MyContainer c;
    c.stored = tid.x;
    outputBuffer[tid.x] = transformElement(c);
}
