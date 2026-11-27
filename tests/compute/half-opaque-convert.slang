//TEST:COMPARE_COMPUTE(targets=cuda):-render-features half
//TEST:COMPARE_COMPUTE(targets=llvm):

//DISABLE_TEST(compute):SIMPLE:-target ptx -stage compute -entry computeMain

// The following test shows if half can be processed as an opaque type. This means
// it can be copied, and converted to and from float types, but nothing else.
// No maths, no swizzling.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    float a = (1.0f / 2.0f) * tid.x;

    // Convert into half
    half ha = f32tof16_(a);
    // Convert back to float
    float fa = f16tof32(ha);

    // Write it out
    outputBuffer[tid] = fa;
}
