//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -compute -entry computeMain -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 6.0
    outputBuffer[0] = (float3x3(15.0) / 2.5)[0][0];
}

