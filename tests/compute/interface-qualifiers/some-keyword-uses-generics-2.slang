//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DEXPLICIT_SPEC

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

__generic<T : IArithmetic>
void logic(out some IFooGeneric<T> dataIn1, some IFooGeneric<T> dataIn2)
{
    dataIn1 = StructForIFooGeneric<T>(0);
    dataIn1.setVal(dataIn2.getVal());
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFooGeneric<float> data1;
    some IFooGeneric<float> data2 = StructForIFooGeneric<float>(1);
#ifdef EXPLICIT_SPEC
    logic<float>(data1, data2);
#else
    logic(data1, data2);
#endif
    buffer[0] = int(data1.getVal() == 
                + data2.getVal());
}