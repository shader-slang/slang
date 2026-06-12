//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type -cpu

// A generic interface can provide a default implementation for a generic
// subscript requirement. The conforming type supplies only `getVal`, so both
// `run` calls below must use the default `__subscript<TIndex>` body from
// `IFoo<int, 7>`.

interface IFoo<TValue, int Offset>
{
    TValue getVal(int index);
    [mutating] void setVal(int index, TValue value);

    __subscript<TIndex>(TIndex index) -> TValue
        where TIndex : IInteger
    {
        get { return getVal(index.toInt() + Offset); }
        set { setVal(index.toInt() + Offset, newValue); }
    }
}

struct Foo : IFoo<int, 7>
{
    int bias;

    int getVal(int index) { return index + bias; }

    [mutating] void setVal(int index, int value) { bias = value - index; }
}

int run<T : IFoo<int, 7>, TIndex : IInteger>(T t, TIndex index)
{
    return t[index];
}

void write<T : IFoo<int, 7>, TIndex : IInteger>(inout T t, TIndex index, int value)
{
    t[index] = value;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f = {};

    outputBuffer[0] = run(f, 100);
    // CHECK: 107

    outputBuffer[1] = run(f, uint(3));
    // CHECK: 10

    write(f, 5, 211);
    outputBuffer[2] = run(f, 5);
    // CHECK: 211

    write(f, uint(6), 307);
    outputBuffer[3] = run(f, uint(6));
    // CHECK: 307
}
