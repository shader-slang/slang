//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

enum E { A=100, B, C }

interface IFoo
{
    int doSomething<T>(T v, E e);

}

//TEST_INPUT: type_conformance FooImpl:IFoo = 1
struct FooImpl : IFoo
{
    int doSomething<T>(T v, E e)
    {
        return (int)e;
    }
}
int helper<T>(IFoo obj, T v, E e)
{
    return obj.doSomething(v, e);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride = 4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain(int id : SV_DispatchThreadID)
{
    var obj = createDynamicObject<IFoo>(id, 0);
    int rs = helper(obj, 42, E.A);
    outputBuffer[0] = rs;
    // CHECK: 100
}
