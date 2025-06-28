//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-vk -compute -xslang -denorm-mode-fp32 -xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=FTZ):-vk -compute -xslang -denorm-mode-fp32 -xslang ftz

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-dx12 -compute -xslang -denorm-mode-fp32 -xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=FTZ):-dx12 -compute -xslang -denorm-mode-fp32 -xslang ftz

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-dx11 -compute -xslang -denorm-mode-fp32 -xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=FTZ):-dx11 -compute -xslang -denorm-mode-fp32 -xslang ftz

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
    float smallNormal = 1.175e-38f; // Smallest normal fp32
    float denorm = smallNormal / 1e6f; // Should be denormal
    result = (denorm != 0.0f) ? 1.0 : 0.0;
    
    outputBuffer[0] = result;
}
