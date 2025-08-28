//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp16 any
//TEST:SIMPLE(filecheck=CHECK_PRESERVE_SPIRV):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp32 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ_SPIRV):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp32 ftz

//TEST:SIMPLE(filecheck=CHECK_ANY):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2
//TEST:SIMPLE(filecheck=CHECK_ANY):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp32 any
//TEST:SIMPLE(filecheck=CHECK_PRESERVE_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp32 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp32 ftz

// Capability shaderDenormPreserveFloat32 is VK_FALSE on the Vulkan device used for CI testing, resulting in
// runtime error VUID-RuntimeSpirv-shaderDenormPreserveFloat32-06297 during CI testing
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-vk -compute -Xslang -denorm-mode-fp32 -Xslang preserve
// Capability shaderDenormFlushToZeroFloat32 is VK_FALSE on the Vulkan device used for CI testing, resulting in
// runtime error VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat32-06300 during CI testing
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=FTZ):-vk -compute -Xslang -denorm-mode-fp32 -Xslang ftz

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-slang -compute -dx12 -profile cs_6_2 -shaderobj -Xslang -denorm-mode-fp32 -Xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=FTZ):-slang -compute -dx12 -profile cs_6_2 -Xslang -denorm-mode-fp32 -Xslang ftz

// CHECK_ANY-NOT: DenormPreserve
// CHECK_ANY-NOT: DenormFlushToZero

// CHECK_PRESERVE_SPIRV: OpExecutionMode %computeMain DenormPreserve 32
// CHECK_PRESERVE_SPIRV-NOT: OpExecutionMode %computeMain DenormPreserve 16
// CHECK_PRESERVE_SPIRV-NOT: OpExecutionMode %computeMain DenormPreserve 64
// CHECK_PRESERVE_SPIRV-NOT: DenormFlushToZero

// CHECK_FTZ_SPIRV: OpExecutionMode %computeMain DenormFlushToZero 32
// CHECK_FTZ_SPIRV-NOT: OpExecutionMode %computeMain DenormFlushToZero 16
// CHECK_FTZ_SPIRV-NOT: OpExecutionMode %computeMain DenormFlushToZero 64
// CHECK_FTZ_SPIRV-NOT: DenormPreserve

// CHECK_ANY-NOT: preserve
// CHECK_ANY-NOT: ftz

// CHECK_PRESERVE_DXIL: attributes #0 = { "fp32-denorm-mode"="preserve" }
// CHECK_PRESERVE_DXIL-NOT: ftz

// CHECK_FTZ_DXIL: attributes #0 = { "fp32-denorm-mode"="ftz" }
// CHECK_FTZ_DXIL-NOT: preserve

// In preserve mode, denormalized numbers should be preserved
// PRESERVE: CCCCD

// In flush-to-zero mode, denormalized numbers should be flushed to zero
// FTZ: 0

// Smallest normal fp32
//TEST_INPUT: set inputBuffer = ubuffer(data=[0x00800000], stride=4)
RWStructuredBuffer<float> inputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float smallestNormal = inputBuffer[0];
    float denormal = smallestNormal / 10;
    
    outputBuffer[0] = denormal;
}
