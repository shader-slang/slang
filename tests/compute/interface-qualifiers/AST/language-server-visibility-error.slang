//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std default

// CHECK: OpEntryPoint
// CHECK-NOT: error 30604

__generic<T : IArithmetic>
interface IFooGeneric
{
    T getVal();
    [mutating]
    void setVal(T inData);
}

__generic<T : IArithmetic>
struct StructForIFooGeneric : IFooGeneric<T>
{
    T v;
    __init(T v_in)
    {
        v = v_in;
    }

    T getVal()
    {
        return v;
    }
    [mutating]
    void setVal(T inData)
    {
        v = inData;
    }
}

RWStructuredBuffer<int> buffer;

void computeMain()
{
    some IFooGeneric<float> data1;
}