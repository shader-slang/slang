//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -dx12 -profile cs_6_0 -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=LIB):-target metallib -entry computeMain -stage compute -DMETAL
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0]):name=uintBuffer
RWByteAddressBuffer uintBuffer;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    uintBuffer.InterlockedAdd(0, 1);
    int oldValue;
    //LIB: call {{.*}}.atomic.global.add.u.i32
    uintBuffer.InterlockedAdd(0, 1, oldValue);
    // BUF: 1
    outputBuffer[0] = oldValue;

    uintBuffer.InterlockedAdd(0, 1, oldValue);
    // BUF: 2
    outputBuffer[1] = (int)oldValue;

    uintBuffer.InterlockedCompareExchange(0, 3, 4, oldValue);
    // BUF: 3
    outputBuffer[2] = (int)oldValue;

    uintBuffer.InterlockedOr(0, 3, oldValue);
    // BUF: 4
    outputBuffer[3] = oldValue; // 4

    uintBuffer.InterlockedExchange(0, 4, oldValue);
    // BUF: 7
    outputBuffer[4] = oldValue; // 7

    uintBuffer.InterlockedMin(0, 3, oldValue);
    // BUF: 4
    outputBuffer[5] = oldValue; // 4

    uintBuffer.InterlockedMax(0, 4, oldValue);
    // BUF: 3
    outputBuffer[6] = oldValue; // 3

    uintBuffer.InterlockedAnd(0, 7, oldValue);
    // BUF: 4
    outputBuffer[7] = oldValue; // 4

    uintBuffer.InterlockedXor(0, 7, oldValue);
    // BUF: 4
    outputBuffer[8] = oldValue; // 4

    // BUF: 3
    outputBuffer[9] = uintBuffer.Load(0);

}
