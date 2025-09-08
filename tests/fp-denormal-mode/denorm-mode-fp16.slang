//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp16 any
//TEST:SIMPLE(filecheck=CHECK_PRESERVE):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp16 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp16 ftz

//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2
//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp16 any
//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp16 preserve
//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp16 ftz

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-vk -compute -Xslang -denorm-mode-fp16 -Xslang preserve
// Capability shaderDenormFlushToZeroFloat16 is VK_FALSE on the Vulkan device used for CI testing, resulting in
// runtime error VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat16-06299 during CI testing
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=FTZ):-vk -compute -Xslang -denorm-mode-fp16 -Xslang ftz

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

// In preserve mode, denormalized numbers should be preserved
// PRESERVE: 66

// In flush-to-zero mode, denormalized numbers should be flushed to zero
// FTZ: 0

// Smallest normal fp16
//TEST_INPUT: set inputBuffer = ubuffer(data=[0x0400], stride=2)
RWStructuredBuffer<half> inputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=2):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    half smallestNormal = inputBuffer[0];
    half denormal = smallestNormal / 10;
    
    outputBuffer[0] = denormal;
}
