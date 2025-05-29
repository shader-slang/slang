// Test that the size of an optional interface type is the same as the existential box.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -profile cs_6_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER): -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER): -vk -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IFoo
{
    int method();
}

//TEST_INPUT: type_conformance Impl1:IFoo = 0
struct Impl1 : IFoo
{
    int data;
    int method() { return data + 1; }
}

struct MyType
{
    Optional<IFoo> foo;
}

Optional<T> process<T>(Optional<T> opt)
{
    return opt;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    MyType t = {};
    t.foo = process(t.foo);

    // BUFFER: 100
    if (let f = t.foo)
        outputBuffer[0] = f.method();
    else
        outputBuffer[0] = 100;
}

// CHECK: struct MyType
// CHECK-NEXT: {
// CHECK-NEXT: Tuple{{.*}} foo{{.*}};