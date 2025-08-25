//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK-ASM): -target metallib
// NOTE: Test not designed for compute, executing for pipecleaning only
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-metal -compute -output-using-type

// CHECK: threadgroup_barrier

// CHECK-ASM: define void @computeMain

//TEST_INPUT: ubuffer(data=[0 0 0], stride=1):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // CHK: 1
    outputBuffer[0] = 1;
    GroupMemoryBarrierWithGroupSync();
    // CHK: 2
    outputBuffer[1] = 2;
    AllMemoryBarrierWithGroupSync();
    // CHK: 3
    outputBuffer[2] = 3;
}
