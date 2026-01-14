//TEST(compute):COMPARE_COMPUTE:-vk -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int sum = 0;

    for (int i = 0; i < 3; ++i) {
        sum += 1;

        switch (i) {
            case 0: {
                continue;
            }
        }
        
        sum += 10;
    }

    outputBuffer[0] = sum;
}
