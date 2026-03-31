//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Scalar math: abs, min, max, clamp
// CHECK: 30
// CHECK: 20
// CHECK: 50
// CHECK: 30
// CHECK: 30
// CHECK: 50

// Vector math: abs, min, max, clamp
// CHECK: 30
// CHECK: 40
// CHECK: 20
// CHECK: 10
// CHECK: 50
// CHECK: 40
// CHECK: 30
// CHECK: 40

[numthreads(1,1,1)]
void computeMain()
{
    // Scalar abs
    BFloat16 absVal = abs(BFloat16(-3.0));
    outputBuffer[0] = int(float(absVal) * 10.0);

    // Scalar min
    BFloat16 minVal = min(BFloat16(2.0), BFloat16(5.0));
    outputBuffer[1] = int(float(minVal) * 10.0);

    // Scalar max
    BFloat16 maxVal = max(BFloat16(2.0), BFloat16(5.0));
    outputBuffer[2] = int(float(maxVal) * 10.0);

    // Scalar clamp (in range)
    BFloat16 clamped = clamp(BFloat16(3.0), BFloat16(1.0), BFloat16(4.0));
    outputBuffer[3] = int(float(clamped) * 10.0);

    // Scalar clamp (below range)
    BFloat16 clampedLow = clamp(BFloat16(0.5), BFloat16(3.0), BFloat16(5.0));
    outputBuffer[4] = int(float(clampedLow) * 10.0);

    // Scalar clamp (above range)
    BFloat16 clampedHigh = clamp(BFloat16(7.0), BFloat16(1.0), BFloat16(5.0));
    outputBuffer[5] = int(float(clampedHigh) * 10.0);

    // Vector abs
    vector<BFloat16, 2> va = vector<BFloat16, 2>(BFloat16(-3.0), BFloat16(-4.0));
    let vabs = abs(va);
    outputBuffer[6] = int(float(vabs[0]) * 10.0);
    outputBuffer[7] = int(float(vabs[1]) * 10.0);

    // Vector min
    vector<BFloat16, 2> vx = vector<BFloat16, 2>(BFloat16(2.0), BFloat16(5.0));
    vector<BFloat16, 2> vy = vector<BFloat16, 2>(BFloat16(4.0), BFloat16(1.0));
    let vmin = min(vx, vy);
    outputBuffer[8] = int(float(vmin[0]) * 10.0);
    outputBuffer[9] = int(float(vmin[1]) * 10.0);

    // Vector max
    let vmax = max(vx, vy);
    outputBuffer[10] = int(float(vmax[0]) * 10.0);
    outputBuffer[11] = int(float(vmax[1]) * 10.0);

    // Vector clamp
    vector<BFloat16, 2> vc = vector<BFloat16, 2>(BFloat16(3.0), BFloat16(6.0));
    vector<BFloat16, 2> vlo = vector<BFloat16, 2>(BFloat16(1.0), BFloat16(2.0));
    vector<BFloat16, 2> vhi = vector<BFloat16, 2>(BFloat16(4.0), BFloat16(4.0));
    let vclamped = clamp(vc, vlo, vhi);
    outputBuffer[12] = int(float(vclamped[0]) * 10.0);
    outputBuffer[13] = int(float(vclamped[1]) * 10.0);
}
