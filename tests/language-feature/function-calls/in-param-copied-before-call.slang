//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// An `in` parameter receives a copy of its argument taken before the call, so it keeps the old
// value when the same variable is also passed to a parameter that the call writes. The struct
// cases are in `in-param-struct-copied-before-call.slang`.

[noinline]
int inThenInout(int a, inout int b)
{
    b = 5;
    return a;
}

[noinline]
int inThenOut(int a, out int b)
{
    b = 5;
    return a;
}

[noinline]
int inThenRef(int a, __ref int b)
{
    b = 5;
    return a;
}

//TEST_INPUT:ubuffer(data=[1 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int x = outputBuffer[0];
    // CHECK: 1
    outputBuffer[0] = inThenInout(x, x);
    // CHECK-NEXT: 5
    outputBuffer[1] = x;

    x = 1;
    // CHECK-NEXT: 1
    outputBuffer[2] = inThenOut(x, x);
    // CHECK-NEXT: 5
    outputBuffer[3] = x;

    x = 1;
    // CHECK-NEXT: 1
    outputBuffer[4] = inThenRef(x, x);
    // CHECK-NEXT: 5
    outputBuffer[5] = x;
}
