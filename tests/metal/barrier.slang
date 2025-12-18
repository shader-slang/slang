//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type

// METAL: threadgroup_barrier

// METALLIB: define void @computeMain

//TEST_INPUT: ubuffer(data=[0 0 0], stride=1):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // BUF: 1.0
    outputBuffer[0] = 1;
    GroupMemoryBarrierWithGroupSync();
    // BUF: 2.0
    outputBuffer[1] = 2;
    AllMemoryBarrierWithGroupSync();
    // BUF: 3.0
    outputBuffer[2] = 3;
}
