// Implicit conversion from a concrete pointer (Foo*) to an interface pointer
// (IFoo*) is not allowed. The type checker requires an explicit cast because
// the memory layouts differ: Foo contains only its fields, while IFoo contains
// a tag, witness table, and AnyValue payload.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

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

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f;
    f.val = 5;
    Foo* pf = &f;

    // Implicit conversion: error
    // CHECK: error[E30019]
    IFoo* ip = pf;

    outputBuffer[0] = 0;
}
