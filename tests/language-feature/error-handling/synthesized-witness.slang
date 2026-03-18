//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError
{
    Fail = 0x10
};

interface IFoo
{
    [mutating]
    int dosomething<T>() throws MyError;
}

struct MyFoo: IFoo
{
    // Note the lack of [mutating] here. This means that the compiler will
    // attempt to synthesize a function that is like:
    //
    // [mutating]
    // int dosomething() throws MyError
    // {
    //     // calls the non-mutating variant
    //     return try dosomething();
    // }
    //
    // This test checks that the 'try' is generated correctly in the synthesized
    // function!
    int dosomething<T>() throws MyError
    {
        return sizeof(T);
    }
}

int helper<T: IFoo>(T t)
{
    do
    {
        return try t.dosomething<int>();
    }
    catch
    {
        return 10;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    MyFoo f;
    // CHECK: 4
    outputBuffer[0] = helper(f);
}

