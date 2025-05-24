//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj

// CHECK: 2
// CHECK-NEXT: 0
// CHECK-NEXT: 11
// CHECK-NEXT: 12
// CHECK-NEXT: 6
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError1
{
    Fail
};

enum MyError2
{
    Fail1,
    Fail2 = 0x12
};

void throwingFunc() throws MyError1
{
    throw MyError1.Fail;
}

int maybeBadFunc1(int n) throws MyError1
{
    if (n == 1) throw MyError1.Fail;
    return n;
}

int maybeBadFunc2(int n) throws MyError2
{
    if (n == 2) throw MyError2.Fail2;
    return n;
}

int multiCatchFunc(int n)
{
    do
    {
        let a = try maybeBadFunc1(n);
        let b = try maybeBadFunc2(n);
        return a+b;
    }
    catch(err: MyError1)
    {
        return 0x11;
    }
    catch(err: MyError2)
    {
        return reinterpret<int>(err);
    }
}

int containedThrow()
{
    do
    {
        throw MyError1.Fail;
    }
    catch(err: MyError1)
    {
        return 1;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    do
    {
        try throwingFunc();
        outputBuffer[0] = 1;
    }
    catch(err: MyError1)
    {
        outputBuffer[0] = 2;
    }

    outputBuffer[1] = multiCatchFunc(0);
    outputBuffer[2] = multiCatchFunc(1);
    outputBuffer[3] = multiCatchFunc(2);
    outputBuffer[4] = multiCatchFunc(3);
    outputBuffer[5] = containedThrow();
}
