// A generic struct parameterized with an interface type preserves
// dynamic dispatch. Wrapper<IFoo> stores the existential and method
// calls on the retrieved value dispatch correctly at runtime.

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

struct Wrapper<T>
{
    T item;
    T getItem() { return item; }
}

IFoo makeObj(int id)
{
    if (id == 0) return FooA();
    else return FooB();
}

float testWrapperStore(int id, float x)
{
    IFoo obj = makeObj(id);
    Wrapper<IFoo> w;
    w.item = obj;
    IFoo retrieved = w.getItem();
    return retrieved.calc(x);
    // REPORT-DAG: note[E50102]: generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float testWrapperDirect(int id, float x)
{
    Wrapper<IFoo> w;
    w.item = makeObj(id);
    return w.item.calc(x);
    // REPORT-DAG: note[E50102]: generated {{.*}} dispatch code{{.*}} 2 possible types:
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // FooA.calc(5.0) = 5.0 * 2.0 = 10.0
    outputBuffer[0] = testWrapperStore(id, 5.0);
    // CHECK: 10.0

    // FooB.calc(5.0) = 5.0 + 10.0 = 15.0
    outputBuffer[1] = testWrapperStore(id + 1, 5.0);
    // CHECK: 15.0

    // Direct field access: FooA.calc(3.0) = 3.0 * 2.0 = 6.0
    outputBuffer[2] = testWrapperDirect(id, 3.0);
    // CHECK: 6.0

    // Direct field access: FooB.calc(3.0) = 3.0 + 10.0 = 13.0
    outputBuffer[3] = testWrapperDirect(id + 1, 3.0);
    // CHECK: 13.0
}
