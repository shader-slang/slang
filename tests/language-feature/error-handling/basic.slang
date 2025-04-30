//TEST(compute):COMPARE_COMPUTE: -shaderobj
//TEST(compute):COMPARE_COMPUTE: -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE: -cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError1
{
    Success,
    Fail
};

enum MyError2
{
    Success,
    Fail
};

extension MyError1 : IError
{
    static MyError1 createSuccessValue() { return Success; }
    bool isSuccessValue() { return this == Success; }
}

extension MyError2 : IError
{
    static MyError2 createSuccessValue() { return Success; }
    bool isSuccessValue() { return this == Success; }
}

/*
void throwingFunc() throws MyError1
{
    throw MyError1.Fail;
}
*/

int maybeBadFunc1(int n) throws MyError1
{
    if (n == 1) throw MyError1.Fail;
    return n;
}

int maybeBadFunc2(int n) throws MyError2
{
    if (n == 2) throw MyError2.Fail;
    return n;
}

int multiCatchFunc(int n)
{
    let a = try maybeBadFunc1(n);
    let b = try maybeBadFunc2(n);
    return a+b;
    catch(err: MyError1)
    {
        return 0x11;
    }
    catch(err: MyError2)
    {
        return 0x12;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        try throwingFunc();
        outputBuffer[0] = 1;
        catch(err: MyError1)
        {
            outputBuffer[0] = 2;
        }
    }

    outputBuffer[1] = multiCatchFunc(0);
    outputBuffer[2] = multiCatchFunc(1);
    outputBuffer[3] = multiCatchFunc(2);
    outputBuffer[4] = multiCatchFunc(3);
}
