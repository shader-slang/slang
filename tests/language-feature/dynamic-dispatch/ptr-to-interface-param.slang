// Passing a pointer-to-interface as a function parameter and dispatching
// through it. This tests that IFoo* survives parameter passing and that
// CastInterfaceToTaggedUnionPtr correctly rewrites the load at the call site.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

interface IFoo
{
    int getVal();
}

struct Foo : IFoo
{
    int val;
    int getVal() { return val * 10; }
}

struct Bar : IFoo
{
    int val;
    int getVal() { return val + 5; }
}

// Dispatch through a pointer parameter.
int callThrough(IFoo* p)
{
    return p->getVal();
}

// Buffer contains a Foo existential: tag=1, data=7.
// Foo::getVal returns 7 * 10 = 70.
//TEST_INPUT: set pFoo = ubuffer(data=[0 0 1 0 7 0 0 0], stride=4);
//TEST_INPUT: type_conformance Foo:IFoo = 1;
//TEST_INPUT: type_conformance Bar:IFoo = 2;
uniform IFoo* pFoo;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Pass the uniform pointer to a function and dispatch through it.
    // CHECK: 70
    outputBuffer[0] = callThrough(pFoo);

    // Direct dereference for comparison.
    // CHECK: 70
    outputBuffer[1] = pFoo->getVal();
}
