//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-vk -compute -xslang -denorm-mode-fp16 -xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=FTZ):-vk -compute -xslang -denorm-mode-fp16 -xslang ftz

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float result = 0.0;
    
    // Create denormal through division
    half smallNormal = half(6.104e-5); // Smallest normal fp16
    float denorm = smallNormal / half(1024.0); // Should be denormal
    result = (denorm != half(0.0)) ? 1.0 : 0.0;
    
    outputBuffer[0] = result;
}

// In preserve mode, denormalized numbers should be preserved (non-zero)
// PRESERVE: 3F800000

// In FTZ mode, denormalized numbers should be flushed to zero
// FTZ: 00000000
