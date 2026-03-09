// A generic method in a struct, constrained with T : IFoo, correctly
// dispatches when called with an interface-typed argument. This works
// even though free generic functions with the same constraint produce
// error 33180.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

interface IFoo
{
    float calc(float x);
}

struct FooA : IFoo
{
    float calc(float x) { return x * 2.0; }
}

struct FooB : IFoo
{
    float calc(float x) { return x + 10.0; }
}

struct Processor
{
    float scale;

    float apply<T : IFoo>(T obj, float x)
    {
        return obj.calc(x) * scale;
        // REPORT-DAG: note[E50102]: generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

IFoo makeObj(int id)
{
    if (id == 0) return FooA();
    else return FooB();
}

float testGenericMethod(int id, float x, float scale)
{
    IFoo obj = makeObj(id);
    Processor p;
    p.scale = scale;
    return p.apply(obj, x);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // FooA: calc(5.0) * 2.0 = 10.0 * 2.0 = 20.0
    outputBuffer[0] = testGenericMethod(id, 5.0, 2.0);
    // CHECK: 20.0

    // FooB: calc(5.0) * 2.0 = 15.0 * 2.0 = 30.0
    outputBuffer[1] = testGenericMethod(id + 1, 5.0, 2.0);
    // CHECK: 30.0

    // FooA: calc(3.0) * 0.5 = 6.0 * 0.5 = 3.0
    outputBuffer[2] = testGenericMethod(id, 3.0, 0.5);
    // CHECK: 3.0

    // FooB: calc(3.0) * 0.5 = 13.0 * 0.5 = 6.5
    outputBuffer[3] = testGenericMethod(id + 1, 3.0, 0.5);
    // CHECK: 6.5
}
