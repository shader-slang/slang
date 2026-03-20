// Casting a concrete-type pointer to an interface pointer requires an explicit
// cast. Implicit conversion from `Foo*` to `IFoo*` is rejected by the type
// checker (error E30019), but explicit C-style cast `(IFoo*)pFoo` succeeds.
//
// The cast is a reinterpret: the pointer bits are preserved. Dispatching
// through the resulting IFoo* only works when the pointed-to memory contains
// a valid serialized existential (tag + AnyValue). When pointing to a plain
// concrete struct, the dispatch reads the struct's memory as if it were an
// existential, producing implementation-defined results.
//
// This test verifies:
// 1. Explicit cast from Foo* to IFoo* compiles (COMPILE check)
// 2. Dispatch through a properly-formatted uniform IFoo* works (runtime check)

// Runtime tests: dispatch through uniform IFoo*.
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

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Dispatch through the uniform pointer.
    // CHECK: 15
    outputBuffer[0] = pFoo->getVal();

    // Explicit cast from concrete pointer to interface pointer compiles.
    // Not dispatched at runtime because the memory layout doesn't match.
    Foo f;
    f.val = 99;
    Foo* pf = &f;
    IFoo* ip = (IFoo*)pf;
    let _ = ip;

    // Second dispatch through uniform pointer.
    // CHECK: 15
    outputBuffer[1] = pFoo->getVal();
}
