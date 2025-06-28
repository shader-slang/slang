//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp64 any
//TEST:SIMPLE(filecheck=CHECK_PRESERVE):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp64 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ):-target spirv-assembly -entry computeMain -stage compute -denorm-mode-fp64 ftz

//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2
//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp64 any
//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp64 preserve
//TEST:SIMPLE(filecheck=CHECK_DXIL):-target dxil-assembly -entry computeMain -stage compute -profile cs_6_2 -denorm-mode-fp64 ftz

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-vk -compute -Xslang -denorm-mode-fp64 -Xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=FTZ):-vk -compute -Xslang -denorm-mode-fp64 -Xslang ftz

// Denormal FP64 values are always preserved by DXC
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-dx12 -compute -Xslang -denorm-mode-fp64 -Xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-dx12 -compute -Xslang -denorm-mode-fp64 -Xslang ftz

// Denormal FP64 values are always preserved by DXC
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-dx11 -compute -Xslang -denorm-mode-fp64 -Xslang preserve
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=PRESERVE):-dx11 -compute -Xslang -denorm-mode-fp64 -Xslang ftz

// CHECK_ANY-NOT: DenormPreserve
// CHECK_ANY-NOT: DenormFlushToZero

// CHECK_PRESERVE: OpExecutionMode %computeMain DenormPreserve 64
// CHECK_PRESERVE-NOT: OpExecutionMode %computeMain DenormPreserve 16
// CHECK_PRESERVE-NOT: OpExecutionMode %computeMain DenormPreserve 32
// CHECK_PRESERVE-NOT: DenormFlushToZero

// CHECK_FTZ: OpExecutionMode %computeMain DenormFlushToZero 64
// CHECK_FTZ-NOT: OpExecutionMode %computeMain DenormFlushToZero 16
// CHECK_FTZ-NOT: OpExecutionMode %computeMain DenormFlushToZero 32
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
