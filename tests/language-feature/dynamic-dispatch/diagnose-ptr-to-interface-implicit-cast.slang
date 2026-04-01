// Covariant pointer conversion (Foo* -> IFoo* when Foo : IFoo) is allowed
// as of #9962. The reverse (contravariant) direction is still rejected.
//
// Note: dereferencing the converted IFoo* and dispatching through it
// (ip->getVal()) is not yet supported when pointing to concrete memory
// rather than a serialized existential. See #10015.

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

    // Covariant direction: Foo* -> IFoo* is allowed (#9962)
    IFoo* ip = pf;

    // Contravariant direction: IFoo* -> Foo* is NOT allowed
    // CHECK: error[E30019]
    Foo* pf2 = ip;

    outputBuffer[0] = 0;
}
