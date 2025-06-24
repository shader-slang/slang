//TEST:SIMPLE(filecheck=CHECK_PRESERVE):-target spirv-assembly -entry main -stage compute -denorm-mode-fp16 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ):-target spirv-assembly -entry main -stage compute -denorm-mode-fp16 ftz

// CHECK_PRESERVE: OpExecutionMode %main DenormPreserve 16
// CHECK_FTZ: OpExecutionMode %main DenormFlushToZero 16

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Use some half operations to ensure floating-point context
    half a = 1.0h;
    half b = 2.0h;
    half result = a + b;
}
