//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type

interface IFoo<T : __BuiltinFloatingPointType>
{
    bool requirement(T v1);
}

struct Bar<T, A : IFoo<T>, B : IFoo<T>> where T : __BuiltinFloatingPointType
{
    [Differentiable]
    T doThing(T a)
    {
        return a * T(2.0);
    }
}

struct Foo : IFoo<float>
{
    bool requirement(float v)
    {
        return v > 0.5;
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(2, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;
    Bar<float, Foo, Foo> obj;
    outputBuffer[tid] = obj.doThing(2.0f);
    // CHECK: 0
    // CHECK: 1
}
