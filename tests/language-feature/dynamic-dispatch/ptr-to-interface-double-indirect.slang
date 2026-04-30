// Double indirection through an interface pointer (IFoo**). Creates IFoo**
// by taking the address of a local IFoo* variable obtained from a uniform
// buffer, then dispatches through double-dereference.
//
// Taking &localExistential (where the local has type IFoo) is not supported
// (see ptr-to-interface-local.slang), but taking & of a local variable of
// type IFoo* works because the address-of operates on the pointer storage
// rather than on an existential value.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

interface IFoo
{
    int getVal();
}

struct Foo : IFoo
{
    int val;
    int getVal() { return val; }
}

//TEST_INPUT: set p = ubuffer(data=[0 0 1 0 42 0 0 0], stride=4);
//TEST_INPUT: type_conformance Foo:IFoo = 1;
uniform IFoo* p;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4);
RWStructuredBuffer<int> outputBuffer;

int dispatchViaDoublePtr(IFoo** pp)
{
    return (*pp)->getVal();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo* localPtr = p;
    // CHECK: 42
    outputBuffer[0] = dispatchViaDoublePtr(__getAddress(localPtr));
}
