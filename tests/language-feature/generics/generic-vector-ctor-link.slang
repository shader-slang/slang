// Regression test for #11182: prelinking an imported generic vector constructor while
// lowering this module used to crash while hoisting a cloned Func type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -profile spirv_1_6 -emit-spirv-directly -stage compute -entry cs_main

import generic_vector_ctor_link_import;

RWStructuredBuffer<float> outputBuffer;

[Differentiable]
vector<T, N> localVectorCtor<T, int N>(vector<T, N> v) where T : __BuiltinFloatingPointType
{
    return vector<T, N>(2.0f);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void cs_main()
{
    var input = diffPair(vector<float, 3>(1.0f), vector<float, 3>(1.0f));
    var importedResult = __fwd_diff(importedVectorCtor<float, 3>)(input);
    var localResult = __fwd_diff(localVectorCtor<float, 3>)(input);
    var siblingResult = importedSiblingCtor<float, 3>(vector<float, 3>(1.0f));
    outputBuffer[0] = importedResult.p.x + importedResult.d.x + localResult.p.x +
                      localResult.d.x + siblingResult.x;
}

// CHECK: OpCapability Shader
// CHECK: OpEntryPoint GLCompute
// CHECK: OpExecutionMode {{.*}} LocalSize 1 1 1
// CHECK: OpTypeVector
// CHECK: OpStore
// CHECK: OpReturn
