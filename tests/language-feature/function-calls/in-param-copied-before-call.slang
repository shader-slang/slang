//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// An `in` parameter receives a copy of its argument taken before the call, so it keeps the old
// value when the same variable is also passed to a parameter that the call writes. The struct
// cases cover targets where `transformParamsToConstRef` turns a struct `in` parameter into a
// borrow; the call site still passes a copy.

struct Big
{
    int v[4];
}

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

[noinline]
int bigInThenInout(Big a, inout Big b)
{
    b.v[0] = 5;
    return a.v[0];
}

[noinline]
int bigInThenOut(Big a, out Big b)
{
    b.v = { 5, 5, 5, 5 };
    return a.v[0];
}

[noinline]
int bigInThenRef(Big a, __ref Big b)
{
    b.v[0] = 5;
    return a.v[0];
}

//TEST_INPUT:ubuffer(data=[1 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
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

    Big big;
    big.v = { 1, 1, 1, 1 };
    // CHECK-NEXT: 1
    outputBuffer[6] = bigInThenInout(big, big);
    // CHECK-NEXT: 5
    outputBuffer[7] = big.v[0];

    big.v = { 1, 1, 1, 1 };
    // CHECK-NEXT: 1
    outputBuffer[8] = bigInThenOut(big, big);
    // CHECK-NEXT: 5
    outputBuffer[9] = big.v[0];

    big.v = { 1, 1, 1, 1 };
    // CHECK-NEXT: 1
    outputBuffer[10] = bigInThenRef(big, big);
    // CHECK-NEXT: 5
    outputBuffer[11] = big.v[0];
}
