// Regression test for #11182: prelinking an imported generic vector constructor while
// lowering this module used to crash while hoisting a cloned Func type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -profile spirv_1_6 -emit-spirv-directly -stage compute -entry cs_main

vector<T, N> f<T, int N>(vector<T, N> v) where T : __BuiltinFloatingPointType
{
    return vector<T, N>(0.0f);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void cs_main()
{
}

// CHECK: OpCapability Shader
// CHECK: OpEntryPoint GLCompute
// CHECK: OpExecutionMode {{.*}} LocalSize 1 1 1
// CHECK: OpReturn
