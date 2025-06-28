//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry genericEntry<half> -stage compute
//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry genericEntry<float> -stage compute
//TEST:SIMPLE(filecheck=CHECK_ANY):-target spirv-assembly -entry genericEntry<double> -stage compute

//TEST:SIMPLE(filecheck=CHECK_PRESERVE):-target spirv-assembly -entry genericEntry<half> -stage compute -denorm-mode-fp32 preserve
//TEST:SIMPLE(filecheck=CHECK_PRESERVE):-target spirv-assembly -entry genericEntry<float> -stage compute -denorm-mode-fp32 preserve
//TEST:SIMPLE(filecheck=CHECK_PRESERVE):-target spirv-assembly -entry genericEntry<double> -stage compute -denorm-mode-fp32 preserve

//TEST:SIMPLE(filecheck=CHECK_FTZ):-target spirv-assembly -entry genericEntry<half> -stage compute -denorm-mode-fp32 ftz
//TEST:SIMPLE(filecheck=CHECK_FTZ):-target spirv-assembly -entry genericEntry<float> -stage compute -denorm-mode-fp32 ftz
//TEST:SIMPLE(filecheck=CHECK_FTZ):-target spirv-assembly -entry genericEntry<double> -stage compute -denorm-mode-fp32 ftz

// CHECK_ANY-NOT: DenormPreserve
// CHECK_ANY-NOT: DenormFlushToZero

// CHECK_PRESERVE: DenormPreserve 32
// CHECK_FTZ: DenormFlushToZero 32

[shader("compute")]  
[numthreads(1, 1, 1)]
__generic<T : __BuiltinArithmeticType> void genericEntry()
{
    T a = T(1);
    T b = T(2);  
    T result = a + b;
}
