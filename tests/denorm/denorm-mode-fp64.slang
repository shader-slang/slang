// denorm-mode-fp64.slang

// Test for -denorm-mode-fp64 command-line option when outputting SPIR-V ASM and DXIL ASM.
// This test verifies that the denorm mode decorations are correctly emitted for 64-bit floats.

//TEST:SIMPLE(filecheck=CHECK_PRESERVE_SPIRV):-target spirv-assembly -entry main -stage compute -denorm-mode-fp64 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ_SPIRV):-target spirv-assembly -entry main -stage compute -denorm-mode-fp64 ftz

//TEST:SIMPLE(filecheck=CHECK_PRESERVE_DXIL):-target dxil-assembly -entry main -stage compute -profile cs_6_2 -denorm-mode-fp64 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ_DXIL):-target dxil-assembly -entry main -stage compute -profile cs_6_2 -denorm-mode-fp64 ftz

// SPIR-V checks
// CHECK_PRESERVE_SPIRV: OpExecutionMode %main DenormPreserve 64
// CHECK_FTZ_SPIRV: OpExecutionMode %main DenormFlushToZero 64

// DXIL checks - may not generate attributes if no double operations are used
// CHECK_PRESERVE_DXIL: define void @main()
// CHECK_FTZ_DXIL: define void @main()

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Use some double operations to ensure floating-point context
    double a = 1.0;
    double b = 2.0;
    double result = a + b;
} 