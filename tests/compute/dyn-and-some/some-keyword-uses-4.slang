//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DIS_SOME
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

#ifdef IS_SOME
#define QUALIFIER some
#else
#define QUALIFIER dyn
#endif


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
    __init(QUALIFIER IFoo v)
    {
        data = v.getVal();
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    // Can coerce `some` to a non `dyn` type
    // assuming we explicitly make a conversion
    // for it.
    //
    // Can coerce `dyn` to a non `dyn` type.

    // CHECK: OpEntryPoint
    // CHECK-NOT: error
    QUALIFIER IFoo data1 = StructForIFoo1(1);
    SomeStruct newStruct = SomeStruct(data1);

    buffer[0] = newStruct.data;
}