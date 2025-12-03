// nested-switch-continue.slang

// Disable FXC DX11 because currently FXC can't compile with 
// "error X3708: 'continue' cannot be used in a switch"
//DISABLE_TEST(compute):COMPARE_COMPUTE:-dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE:-dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE:-vk -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE:-wgpu

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

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[0] = testWhile(0);
    outputBuffer[1] = testWhile(1);
}
