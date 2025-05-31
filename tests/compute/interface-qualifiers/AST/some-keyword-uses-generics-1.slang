//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// CHECK: OpEntryPoint
// CHECK-NOT: error

__generic<T : IArithmetic>
interface IFooGeneric
{
    bool getVal();
}

__generic<T : IArithmetic>
struct StructForIFooGeneric : IFooGeneric<T>
{
    T v;
    __init(T v_in)
    {
        v = v_in;
    }

    bool getVal()
    {
        return v == T(0);
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{

    some IFooGeneric<float> data1;
    data1 = StructForIFooGeneric<float>(1);
    some IFooGeneric<float> data2 = StructForIFooGeneric<float>(1);

    buffer[0] = data1.getVal() 
                + data2.getVal() + 2;
}