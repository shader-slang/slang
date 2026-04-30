//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx11 -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

Conditional<int, true> returnConditionalTrue(int val)
{
    return val;
}

Conditional<int, false> returnConditionalFalse(int unused)
{
    return unused;
}

// CHECK: 42
// CHECK-NEXT: -1
// CHECK-NEXT: 10
// CHECK-NEXT: -1
[numthreads(1, 1, 1)]
void computeMain()
{
    Conditional<int, true> ct = returnConditionalTrue(42);
    if (let v = ct.get())
        outputBuffer[0] = v;
    else
        outputBuffer[0] = -1;

    Conditional<int, false> cf = returnConditionalFalse(99);
    if (let v = cf.get())
        outputBuffer[1] = v;
    else
        outputBuffer[1] = -1;

    Conditional<int, true> direct = 10;
    if (let v = direct.get())
        outputBuffer[2] = v;
    else
        outputBuffer[2] = -1;

    Conditional<int, false> empty;
    if (let v = empty.get())
        outputBuffer[3] = v;
    else
        outputBuffer[3] = -1;
}
