//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp16 any
//TEST:SIMPLE(filecheck=CHECK_PRESERVE):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp16 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp16 ftz

//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2
//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp16 any
//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp16 preserve
//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp16 ftz

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-vk -compute -Xslang -denorm-mode-fp16 -Xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=FTZ):-vk -compute -Xslang -denorm-mode-fp16 -Xslang ftz

// Denormal FP16 values are always preserved by DXC
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-dx12 -compute -Xslang -denorm-mode-fp16 -Xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-dx12 -compute -Xslang -denorm-mode-fp16 -Xslang ftz

// Denormal FP16 values are always preserved by DXC
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-dx11 -compute -Xslang -denorm-mode-fp16 -Xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-dx11 -compute -Xslang -denorm-mode-fp16 -Xslang ftz

// CHECK_ANY-NOT: DenormPreserve
// CHECK_ANY-NOT: DenormFlushToZero

// CHECK_PRESERVE: OpExecutionMode %computeMain DenormPreserve 16
// CHECK_PRESERVE-NOT: OpExecutionMode %computeMain DenormPreserve 32
// CHECK_PRESERVE-NOT: OpExecutionMode %computeMain DenormPreserve 64
// CHECK_PRESERVE-NOT: DenormFlushToZero

// CHECK_FTZ: OpExecutionMode %computeMain DenormFlushToZero 16
// CHECK_FTZ-NOT: OpExecutionMode %computeMain DenormFlushToZero 32
// CHECK_FTZ-NOT: OpExecutionMode %computeMain DenormFlushToZero 64
// CHECK_FTZ-NOT: DenormPreserve

// CHECK_DXIL-NOT: fp32-denorm-mode
// CHECK_DXIL-NOT: preserve
// CHECK_DXIL-NOT: ftz

// In preserve mode, denormalized numbers should be preserved (non-zero)
// PRESERVE: 3F800000

// In FTZ mode, denormalized numbers should be flushed to zero
// FTZ: 00000000

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float result = 0.0;
    
    // Create denormal through division
    half smallNormal = half(6.104e-5); // Smallest normal fp16
    float denorm = smallNormal / half(1024.0); // Should be denormal
    result = (denorm != half(0.0)) ? 1.0 : 0.0;
    
    outputBuffer[0] = result;
}
