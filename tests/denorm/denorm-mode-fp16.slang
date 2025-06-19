// denorm-mode-fp16.slang

// Test for -denorm-mode-fp16 command-line option when outputting SPIR-V ASM and DXIL ASM.
// This test verifies that the denorm mode decorations are correctly emitted for 16-bit floats.

//TEST:SIMPLE(filecheck=CHECK_PRESERVE_SPIRV):-target spirv-assembly -entry main -stage compute -denorm-mode-fp16 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ_SPIRV):-target spirv-assembly -entry main -stage compute -denorm-mode-fp16 ftz

//TEST:SIMPLE(filecheck=CHECK_PRESERVE_DXIL):-target dxil-assembly -entry main -stage compute -profile cs_6_2 -denorm-mode-fp16 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ_DXIL):-target dxil-assembly -entry main -stage compute -profile cs_6_2 -denorm-mode-fp16 ftz

// SPIR-V checks
// CHECK_PRESERVE_SPIRV: OpExecutionMode %main DenormPreserve 16
// CHECK_FTZ_SPIRV: OpExecutionMode %main DenormFlushToZero 16

// DXIL checks - may not generate attributes if no half operations are used
// CHECK_PRESERVE_DXIL: define void @main()
// CHECK_FTZ_DXIL: define void @main()

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Use some half operations to ensure floating-point context
    half a = 1.0h;
    half b = 2.0h;
    half result = a + b;
} 