// A member of an extension on a pointer to an interface type is called on the pointer, so
// the pointer expression is evaluated once. A member that the opened value has, either an
// interface requirement or a member of `extension<T : IFoo> T`, still wins over a
// pointer-extension member with the same name.

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

// The buffer holds a serialized `Bar` existential (tag 2, value 2.0f), so `getVal` returns 3.
//TEST_INPUT: set pFoo = ubuffer(data=[0 0 2 0 2.0f], stride=4);
//TEST_INPUT: type_conformance Foo:IFoo = 1;
//TEST_INPUT: type_conformance Bar:IFoo = 2;
uniform IFoo* pFoo;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

extension<T : IFoo> T
{
    int extra() { return 7; }
}

extension<T> Ptr<T>
{
    int tag() { return 10; }
    int getVal() { return -1; }
    int extra() { return -1; }
}

static int calls = 0;

IFoo* nextEntry()
{
    calls++;
    return pFoo;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 10
    outputBuffer[0] = nextEntry().tag();
    // CHECK-NEXT: 1
    outputBuffer[1] = calls;
    // CHECK-NEXT: 3
    outputBuffer[2] = nextEntry().getVal();
    // CHECK-NEXT: 2
    outputBuffer[3] = calls;
    // CHECK-NEXT: 10
    outputBuffer[4] = pFoo.tag();
    // CHECK-NEXT: 7
    outputBuffer[5] = pFoo.extra();
}
