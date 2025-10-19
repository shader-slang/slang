//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Foo
{
    int8_t a;
    double b;
};

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Foo f = {0, 0.0};
    // CHECK: 8
    outputBuffer[0] = __alignOf<Foo>();
    // CHECK-NEXT: 8
    outputBuffer[1] = __alignOf(f);
    // CHECK-NEXT: 10
    outputBuffer[2] = __sizeOf<Foo>();
    // CHECK-NEXT: 10
    outputBuffer[3] = __sizeOf(f);
}
