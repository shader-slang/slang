// nested-switch.slang

//TEST(compute):COMPARE_COMPUTE: -shaderobj
//TEST(compute):COMPARE_COMPUTE:-vk -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE:-wgpu

int test(int t, int r)
{
    int result = 0;
    switch (t)
    {
    case 0:
        switch (r)
        {
        case 0:
            return 1;
        case 2:
            return 2;
        }
    default:
        result = 3;
        break;
    }
    return result;
}

int testWhile(int r) {
    int result = 0;
    int i = 0;
    while (i < 3) {
        i += 1;
        switch (r) {
        case 0:
            // continue to while loop, not increment result
            continue;
        default:
            // should only exit the switch,
            // will execute the remaining code in the while loop
            break;
        }
        result += 1;
    }
    return result;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[0] = test(dispatchThreadID.x, 0);
    outputBuffer[1] = test(dispatchThreadID.x, 1);
    outputBuffer[2] = test(dispatchThreadID.x, 2);
    outputBuffer[3] = testWhile(0);
    outputBuffer[4] = testWhile(1);
}
