//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj

// CHECK: error SOME_ID

__generic<T : IArithmetic>
interface IFoo
{
    T getVal();
    [mutating]
    void setVal(T v);
}

__generic<T : IArithmetic>
struct StructForIFoo1 : IFoo<T>
{
    __init(T v)
    {
        a = v;
    }
    T a;
    T getVal()
    {
        return a;
    }
    [mutating]
    void setVal(T v)
    {
        a = v;
    }
};
__generic<T : IArithmetic>
struct StructForIFoo2 : IFoo<T>
{
    __init(T v)
    {
        a = v+(T)10;
    }
    T a;
    T getVal()
    {
        return a;
    }
    [mutating]
    void setVal(T v)
    {
        a = v;
    }
};

__generic<T : IArithmetic>
some IFoo<T> returnIFoo(in some IFoo<T> data1, in some IFoo<T> data2)
{
    // Potentially returning multiple different types, error
    if (data1.getVal() != (T)0)
    {
        return data1;
    }
    else
    {
        data1.setVal((T)5);
        return data1;
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

void computeMain()
{
    some IFoo<int> data1 = StructForIFoo1<int>(1);
    some IFoo<int> data2 = StructForIFoo1<int>(2);
    returnIFoo<int>(data1, data2);
//CHECK: 1
    outputBuffer[0] = data1.getVal();
}