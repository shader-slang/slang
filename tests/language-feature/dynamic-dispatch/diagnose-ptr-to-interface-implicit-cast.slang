// Implicit pointer conversion is rejected when the in-memory representations
// are incompatible. This covers:
// 1. Concrete struct pointer to interface pointer (layouts differ)
// 2. Non-conforming type pointer to interface pointer
// 3. Access qualifier mismatch between interface pointers

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

struct Bar
{
    int val;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f;
    f.val = 5;
    Foo* pf = &f;

    // Concrete-to-interface: rejected (memory layouts differ)
    // CHECK: error[E30019]
    IFoo* ip = pf;

    // Non-conforming type: rejected (Bar does not implement IFoo)
    Bar b;
    b.val = 1;
    // CHECK: error[E30019]
    IFoo* ip2 = &b;

    // Access qualifier mismatch: rejected
    Ptr<IFoo, Access.Read> pRead;
    // CHECK: error[E30019]
    Ptr<IFoo, Access.ReadWrite> pWrite = pRead;

    outputBuffer[0] = 0;
}
