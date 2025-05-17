//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj

// CHECK: 5
// CHECK-NEXT: 1
// CHECK-NEXT: 0

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
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

T func<T: __BuiltinFloatingPointType>(T val) throws MyError
{
    if (val >= T(3))
        throw MyError.Fail;
    return val * T(2);
    catch(err: MyError)
    {
        // Just rethrow to test catching inside a generic as well.
        throw err;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int i = 0;
    {
        outputBuffer[i] = int(try func(2.5f));
        i+=1;
        outputBuffer[i] = int(try func(3.5f));
        i+=1;

        catch(err: MyError)
        {
            outputBuffer[i] = reinterpret<int>(err);
        }
    }
}
