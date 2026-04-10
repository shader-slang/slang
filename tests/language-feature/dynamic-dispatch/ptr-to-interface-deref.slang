// Dereferencing a uniform pointer to an interface type and dispatching a method
// through it. The pointer targets a buffer containing a serialized existential
// value (tag + data), and the compiler rewrites the load via
// CastInterfaceToTaggedUnionPtr to recover the concrete type at runtime.
//
// The existing existential-ptr.slang test covers Vulkan only. This test extends
// coverage to additional backends.

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
    int getVal() { return val; }
}

struct Bar : IFoo
{
    float val;
    int getVal() { return (int)val + 1; }
}

// Buffer encodes a serialized Bar existential: tag=2, data=2.0f.
// Bar::getVal returns (int)2.0 + 1 = 3.
//TEST_INPUT: set pFoo = ubuffer(data=[0 0 2 0 2.0f], stride=4);
//TEST_INPUT: type_conformance Foo:IFoo = 1;
//TEST_INPUT: type_conformance Bar:IFoo = 2;
uniform IFoo* pFoo;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Dereference the pointer and dispatch through the interface.
    // CHECK: 3
    outputBuffer[0] = pFoo->getVal();
}
