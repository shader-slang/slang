// Double indirection through an interface pointer (IFoo**). The marshalling
// code handles IFoo** as raw pointer bits (PtrType case), but
// CastInterfaceToTaggedUnionPtr only supports single indirection (IFoo*).
// This test checks whether double indirection compiles to valid output.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

interface IFoo
{
    int getVal();
}

struct Foo : IFoo
{
    int val;
    int getVal() { return val; }
}

//TEST_INPUT: type_conformance Foo:IFoo = 1;

RWStructuredBuffer<int> outputBuffer;

// CHECK: computeMain
[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f;
    f.val = 42;
    IFoo ifoo = f;
    IFoo* p = &ifoo;
    IFoo** pp = &p;

    outputBuffer[0] = (*pp)->getVal();
}
