//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// A struct `in` parameter receives a copy of its argument taken before the call, so it keeps the
// old value when the same variable is also passed to a parameter that the call writes. These cases
// cover targets where `transformParamsToConstRef` turns a struct `in` parameter into a borrow; the
// call site still passes a copy. The callees are `[noinline]` so the borrow survives to emission.
// D3D12 is not tested because DXIL rejects a struct passed by value to a function that is not
// inlined.

struct Big
{
    int v[4];
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

//TEST_INPUT:ubuffer(data=[1 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Big big;
    big.v = { 1, 1, 1, 1 };
    // CHECK: 1
    outputBuffer[0] = bigInThenInout(big, big);
    // CHECK-NEXT: 5
    outputBuffer[1] = big.v[0];

    big.v = { 1, 1, 1, 1 };
    // CHECK-NEXT: 1
    outputBuffer[2] = bigInThenOut(big, big);
    // CHECK-NEXT: 5
    outputBuffer[3] = big.v[0];

    big.v = { 1, 1, 1, 1 };
    // CHECK-NEXT: 1
    outputBuffer[4] = bigInThenRef(big, big);
    // CHECK-NEXT: 5
    outputBuffer[5] = big.v[0];
}
