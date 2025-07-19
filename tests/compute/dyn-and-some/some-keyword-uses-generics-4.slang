//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// CHECK: OpEntryPoint
// CHECK-NOT: error

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


__generic<T : IArithmetic>
some IFooGeneric<T> logic1()
{
    return StructForIFooGeneric<T>(1);
}

void computeMain()
{
    some IFooGeneric<float> data1 = logic1<float>();

    buffer[0] = int(data1.getVal() == 1);
}