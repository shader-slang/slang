//TEST:SIMPLE(filecheck=CHECK_SOME): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DIS_SOME
//TEST:SIMPLE(filecheck=CHECK_DYN): -target spirv -stage compute -entry computeMain -lang slang -std 2026

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
    __implicit_conversion(1)
    __init(QUALIFIER IFoo v)
    {
        data = v.getVal();
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    // Cannot coerce `some` to a non `dyn` type
    // assuming we implicitly make a conversion
    // for it. This is because implicit conversion
    // requires exactly the same type to overload
    // and 2 different `some` types are never equal.
    //
    // Can coerce `dyn` to a non `dyn` type.

    // CHECK_SOME: error 30019

    // CHECK_DYN: OpEntryPoint
    // CHECK_DYN-NOT: error
    QUALIFIER IFoo data1 = StructForIFoo1(1);
    SomeStruct newStruct = data1;

    buffer[0] = newStruct.data;
}