//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-vk -compute -xslang -denorm-mode-fp64 -xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=FTZ):-vk -compute -xslang -denorm-mode-fp64 -xslang ftz

// In preserve mode, denormalized numbers should be preserved (non-zero)
// PRESERVE: 3F800000

// In FTZ mode, denormalized numbers should be flushed to zero
// FTZ: 00000000

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float result = 0.0;
    
    // Create denormal through division
    double smallNormal = double(2.22e-38); // Small normal fp32
    double denorm = smallNormal;
    int exp = -38;
    // Using a small normal fp64 for smallNormal results in a warning about the literal being too small for a float
    // Divide until we get a subnormal value
    while (exp > -308) {
        denorm = denorm / double(1e20);
        exp -= 20;
    }
    result = (denorm != 0.0) ? 1.0 : 0.0;
    
    outputBuffer[0] = result;
}
