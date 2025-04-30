//TEST(compute):COMPARE_COMPUTE: -shaderobj
//TEST(compute):COMPARE_COMPUTE: -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE: -cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct MyError
{
    int code = 0;
    int param = 0;
};

extension MyError : IError
{
    static MyError createSuccessValue()
    {
        return MyError(0, 0);
    }

    bool isSuccessValue()
    {
        return code == 0;
    }
}

int func(int val) throws MyError
{
    if (val >= 3)
        throw MyError(1, val);
    return val * 2;
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int i = 0;
    {
        outputBuffer[i] = try func(1);
        i+=1;
        outputBuffer[i] = try func(3);
        i+=1;

        catch(err: MyError)
        {
            outputBuffer[i] = err.code * 0x10 + err.param;
        }
    }
}
