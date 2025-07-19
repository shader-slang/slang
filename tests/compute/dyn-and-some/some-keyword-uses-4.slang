//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

dyn interface IFoo
{
    int getVal();
}

struct StructForIFoo1 : IFoo
{
    __init(int v)
    {
        a = v;
    }
    int a = 0;
    int getVal()
    {
        return a;
    }
};

struct SomeStruct
{
    int data;
    __init(inout some IFoo v1, inout dyn IFoo v2)
    {
        data = v1.getVal() + v2.getVal();
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    // CHECK: OpEntryPoint
    // CHECK-NOT: error
    some IFoo data1 = StructForIFoo1(1);
    dyn IFoo data2 = StructForIFoo1(1);
    SomeStruct newStruct = SomeStruct(data1, data2);

    buffer[0] = newStruct.data;
}