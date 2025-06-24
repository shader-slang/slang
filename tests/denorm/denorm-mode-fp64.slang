//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry main -stage compute -denorm-mode-fp64 any
//TEST:SIMPLE(filecheck=CHECK_PRESERVE):-target spirv-assembly -entry main -stage compute -denorm-mode-fp64 preserve
//TEST:SIMPLE(filecheck=CHECK_FTZ):-target spirv-assembly -entry main -stage compute -denorm-mode-fp64 ftz

// CHECK_ANY-NOT: DenormPreserve
// CHECK_ANY-NOT: DenormFlushToZero
// CHECK_PRESERVE: OpExecutionMode %main DenormPreserve 64
// CHECK_FTZ: OpExecutionMode %main DenormFlushToZero 64

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Use some double operations to ensure floating-point context
    double a = 1.0;
    double b = 2.0;
    double result = a + b;
}
