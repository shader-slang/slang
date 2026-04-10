// Implicit conversion from a concrete pointer (Foo*) to an interface pointer
// (IFoo*) is not allowed because the pointed-to data layouts are not bit
// equivalent. The compiler emits a specific diagnostic explaining this.

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

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f;
    f.val = 5;
    Foo* pf = &f;

    // Concrete-to-interface pointer: rejected with layout explanation
    // CHECK: error[E30019]
    // CHECK: not bit equivalent
    IFoo* ip = pf;

    outputBuffer[0] = 0;
}
