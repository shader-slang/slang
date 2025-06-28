//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry main -stage compute -denorm-mode-fp16 any
//TEST:SIMPLE(filecheck=CHECK_PRESERVE_SPIRV):-target spirv-assembly -entry main -stage compute -denorm-mode-fp32 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ_SPIRV):-target spirv-assembly -entry main -stage compute -denorm-mode-fp32 ftz

//TEST:SIMPLE(filecheck=CHECK_ANY):-target dxil-assembly -entry main -stage compute -profile cs_6_2
//TEST:SIMPLE(filecheck=CHECK_ANY):-target dxil-assembly -entry main -stage compute -profile cs_6_2 -denorm-mode-fp32 any
//TEST:SIMPLE(filecheck=CHECK_PRESERVE_DXIL):-target dxil-assembly -entry main -stage compute -profile cs_6_2 -denorm-mode-fp32 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ_DXIL):-target dxil-assembly -entry main -stage compute -profile cs_6_2 -denorm-mode-fp32 ftz

// CHECK_ANY-NOT: DenormPreserve
// CHECK_ANY-NOT: DenormFlushToZero

// CHECK_PRESERVE_SPIRV: OpExecutionMode %main DenormPreserve 32
// CHECK_PRESERVE_SPIRV-NOT: OpExecutionMode %main DenormPreserve 16
// CHECK_PRESERVE_SPIRV-NOT: OpExecutionMode %main DenormPreserve 64
// CHECK_PRESERVE_SPIRV-NOT: DenormFlushToZero

// CHECK_FTZ_SPIRV: OpExecutionMode %main DenormFlushToZero 32
// CHECK_FTZ_SPIRV-NOT: OpExecutionMode %main DenormFlushToZero 16
// CHECK_FTZ_SPIRV-NOT: OpExecutionMode %main DenormFlushToZero 64
// CHECK_FTZ_SPIRV-NOT: DenormPreserve

// CHECK_ANY-NOT: preserve
// CHECK_ANY-NOT: ftz

// CHECK_PRESERVE_DXIL: attributes #0 = { "fp32-denorm-mode"="preserve" }
// CHECK_PRESERVE_DXIL-NOT: ftz

// CHECK_FTZ_DXIL: attributes #0 = { "fp32-denorm-mode"="ftz" }
// CHECK_FTZ_DXIL-NOT: preserve

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Use some float operations to ensure floating-point context
    float a = 1.0f;
    float b = 2.0f;
    float result = a + b;
}
