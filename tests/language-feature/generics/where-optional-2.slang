//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IThing
{
    [mutating]
    int thing(int index);
}

struct MyThing: IThing
{
    int val;

    [mutating]
    int thing(int index)
    {
        val++;
        outputBuffer[index] = val;
        return val;
    }
}

struct NotMyThing
{
    int val;
}

void f<T>(inout T t, int index) where optional T: IThing
{
    if (T is IThing)
    {
        outputBuffer[index+1] = 2 * t.thing(index);
    }
    else
    {
        outputBuffer[index] = 0;
        outputBuffer[index+1] = 0;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    MyThing mt = MyThing(0);
    NotMyThing nt = NotMyThing(1);

    // CHECK: 1
    // CHECK-NEXT: 2
    f<MyThing>(mt, 0);
    // CHECK-NEXT: 0
    // CHECK-NEXT: 0
    f<NotMyThing>(nt, 2);
    // CHECK: 2
    // CHECK-NEXT: 4
    f(mt, 4);
    // CHECK-NEXT: 0
    // CHECK-NEXT: 0
    f(nt, 6);
}
