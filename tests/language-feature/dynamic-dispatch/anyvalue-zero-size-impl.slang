// Dynamic dispatch with an empty struct (zero data bytes) conforming to an
// interface. The compiler must handle AnyValue packing for a type whose payload
// is zero bytes alongside a type with actual data.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IFoo
{
    int getValue();
}

struct EmptyImpl : IFoo
{
    int getValue() { return 42; }
}

struct DataImpl : IFoo
{
    int v;
    int getValue() { return v; }
}

IFoo createFoo(int id)
{
    if (id == 0)
    {
        EmptyImpl e;
        return e;
    }
    else
    {
        DataImpl d;
        d.v = 99;
        return d;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var a = createFoo(0);
    outputBuffer[0] = a.getValue();
    // CHECK: 42

    var b = createFoo(1);
    outputBuffer[1] = b.getValue();
    // CHECK: 99
}
