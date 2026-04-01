// Pointer-to-interface cast: both implicit covariant conversion (Foo* -> IFoo*
// when Foo : IFoo, #9962) and dispatch through a properly-formatted uniform
// IFoo* are tested at runtime.
//
// The cast is a bitcast: the pointer bits are preserved. Dereferencing the
// resulting IFoo* and dispatching through it (ip->getVal()) is NOT supported
// when the pointed-to memory contains a plain concrete struct rather than a
// serialized existential (tag + witness table + AnyValue). That is tracked
// separately in #10015.

// Runtime tests: dispatch through uniform IFoo* and implicit covariant cast.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

interface IFoo
{
    int getVal();
}

struct Foo : IFoo
{
    int val;
    int getVal() { return val * 3; }
}

struct Bar : IFoo
{
    int val;
    int getVal() { return val + 100; }
}

// Buffer encodes a Foo existential: tag=1, data=5.
// Foo::getVal returns 5 * 3 = 15.
//TEST_INPUT: set pFoo = ubuffer(data=[0 0 1 0 5 0 0 0], stride=4);
//TEST_INPUT: type_conformance Foo:IFoo = 1;
//TEST_INPUT: type_conformance Bar:IFoo = 2;
uniform IFoo* pFoo;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Dispatch through the uniform pointer.
    // CHECK: 15
    outputBuffer[0] = pFoo->getVal();

    // Implicit covariant conversion: Foo* -> IFoo* (#9962).
    // The pointer value (address) is preserved via bitcast.
    Foo f;
    f.val = 99;
    Foo* pf = &f;
    IFoo* ip = pf;

    // Verify the converted pointer is non-null (address was preserved).
    // CHECK: 1
    outputBuffer[1] = (ip != nullptr) ? 1 : 0;

    // Implicit covariant conversion from a different conforming type.
    Bar b;
    b.val = 50;
    IFoo* ip2 = &b;

    // CHECK: 1
    outputBuffer[2] = (ip2 != nullptr) ? 1 : 0;

    // Second dispatch through uniform pointer.
    // CHECK: 15
    outputBuffer[3] = pFoo->getVal();
}
