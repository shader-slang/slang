//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError
{
    Success,
    Fail
};

extension MyError : IError
{
    static MyError createSuccessValue() { return Success; }
    bool isSuccessValue() { return this == Success; }
}

    
int maybeThrowingFunc(int n) throws MyError
{
    if (n == 3)
        throw MyError.Fail;
    return n;
}

void testFunc(int n, inout int i)
{
    int value = n;
    defer
    {
        outputBuffer[i++] = value;
    }

    defer
    {
        let m = try maybeThrowingFunc(n);
        value += m;
        catch(err: MyError)
        {
            defer
            {
                outputBuffer[i++] = 0x80;
            }
            outputBuffer[i++] = 0xFF;
        }
    }
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int i = 0;
    // CHECK: 2
    testFunc(1, i);
    // CHECK-NEXT: 4
    testFunc(2, i);
    // CHECK-NEXT: FF
    // CHECK-NEXT: 80
    // CHECK-NEXT: 3
    testFunc(3, i);
}
