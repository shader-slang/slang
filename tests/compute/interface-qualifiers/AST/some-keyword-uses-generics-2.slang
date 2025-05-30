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
void logicInner1(out some IFooGeneric<T> dataIn1)
{
    dataIn1 = StructForIFooGeneric<T>(0);
}

__generic<T : IArithmetic>
void logic1(out some IFooGeneric<T> dataIn1)
{
    logicInner1<T>(dataIn1);
}

void computeMain()
{
    some IFooGeneric<float> data1;
    some IFooGeneric<float> data2 = StructForIFooGeneric<float>(1);
    logic1<float>(data1);
    buffer[0] = int(data1.getVal() == 
                + data2.getVal());
}