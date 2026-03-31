//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// CHECK: 30
// CHECK: 20
// CHECK: 50
// CHECK: 30
// CHECK: 30
// CHECK: 50

[numthreads(1,1,1)]
void computeMain()
{
    BFloat16 a = BFloat16(-3.0);
    BFloat16 b = BFloat16(2.0);
    BFloat16 c = BFloat16(5.0);

    // abs
    BFloat16 absVal = abs(a);
    outputBuffer[0] = int(float(absVal) * 10.0);

    // min
    BFloat16 minVal = min(BFloat16(2.0), BFloat16(5.0));
    outputBuffer[1] = int(float(minVal) * 10.0);

    // max
    BFloat16 maxVal = max(BFloat16(2.0), BFloat16(5.0));
    outputBuffer[2] = int(float(maxVal) * 10.0);

    // clamp
    BFloat16 clamped = clamp(BFloat16(3.0), BFloat16(1.0), BFloat16(4.0));
    outputBuffer[3] = int(float(clamped) * 10.0);

    // clamp at lower bound
    BFloat16 clampedLow = clamp(BFloat16(0.5), BFloat16(3.0), BFloat16(5.0));
    outputBuffer[4] = int(float(clampedLow) * 10.0);

    // clamp at upper bound
    BFloat16 clampedHigh = clamp(BFloat16(7.0), BFloat16(1.0), BFloat16(5.0));
    outputBuffer[5] = int(float(clampedHigh) * 10.0);
}
