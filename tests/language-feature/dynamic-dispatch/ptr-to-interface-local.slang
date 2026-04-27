// Taking the address of a local interface-typed variable is not supported.
// The existential value has an internal representation (tagged union / AnyValue)
// whose pointer type does not match IFoo*. The address-of operation is rejected
// because the operand's internal type differs from the declared interface type.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

interface IFoo
{
    int getVal();
}

struct Alpha : IFoo
{
    int x;
    int getVal() { return x + 1; }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Alpha a;
    a.x = 9;
    IFoo local = a;

    // CHECK: error[E30019]
    // CHECK: error[E31207]
    IFoo* p = &local;
}
