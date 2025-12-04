//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface I
{
}

struct A<T> where optional T: I
{

    int f()
    {
        if (T is I)
            return 0;
        else
            return 1;
    }
}

struct B<T>
{
    // A NoneWitness is created in the IR for specializing A without satisfying
    // I. This NoneWitness should be hoisted to the global scope; otherwise it
    // gets generated _after_ the terminator of the outer generic block,
    // breaking a whole lot of stuff.
    A<T> a;
    A<T> b;
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    B<float> b;
    // CHECK: 1
    // CHECK-NEXT: 2
    outputBuffer[0] = b.a.f();
    outputBuffer[1] = b.b.f()*2;
}
