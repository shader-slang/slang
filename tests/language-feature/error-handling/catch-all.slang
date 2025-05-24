//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyError1
{
    Fail = 0x10
};

enum MyError2
{
    Fail = 0x20
};

int f(int n) throws MyError1
{
    if (n == 1) throw MyError1.Fail;
    return n;
}

int g(int n) throws MyError2
{
    if (n == 2) throw MyError2.Fail;
    return n;
}

void handlerFunc(int i, int n)
{
    do
    {
        int a = try f(n);
        int b = try g(n);
        int c = a+b+1;
        outputBuffer[i] = c;
    }
    catch(err: MyError1)
    {
        outputBuffer[i] = reinterpret<int>(err);
    }
    catch
    {
        outputBuffer[i] = 0x30;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int i = 0;
    handlerFunc(0, 0); // CHECK: 1
    handlerFunc(1, 1); // CHECK-NEXT: 10
    handlerFunc(2, 2); // CHECK-NEXT: 30
    handlerFunc(3, 3); // CHECK-NEXT 7
}
