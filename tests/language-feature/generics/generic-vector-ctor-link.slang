// Regression test for #11182: prelinking an imported generic vector constructor while
// lowering this module used to crash while hoisting a cloned Func type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -profile spirv_1_6 -emit-spirv-directly -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_RUN):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_RUN):-cpu -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK_RUN):-vk -compute -shaderobj -output-using-type

import generic_vector_ctor_link_import;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
vector<T, N> localVectorCtor<T, int N>(vector<T, N> v) where T : __BuiltinFloatingPointType
{
    return vector<T, N>(2.0f);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    var input = diffPair(vector<float, 3>(1.0f), vector<float, 3>(1.0f));
    var importedResult = __fwd_diff(importedVectorCtor<float, 3>)(input);
    var localResult = __fwd_diff(localVectorCtor<float, 3>)(input);
    var siblingResult = importedSiblingCtor<float, 3>(vector<float, 3>(1.0f));
    var result = importedResult.p.x + importedResult.d.x + localResult.p.x + localResult.d.x +
                 siblingResult.x;

    outputBuffer[0] = result; // CHECK_RUN: 3.0
}

// CHECK: OpCapability Shader
// CHECK: OpEntryPoint GLCompute
// CHECK: OpExecutionMode {{.*}} LocalSize 1 1 1
// CHECK: ; Types, variables and constants
// CHECK-COUNT-1: OpTypeFunction
// CHECK-NOT: OpTypeFunction
// CHECK: OpTypeFloat 32
// CHECK-NOT: OpTypeFunction
// CHECK-COUNT-1: OpTypeVector %float 3
// CHECK-NOT: OpTypeFunction
// CHECK-NOT: OpTypeVector %float 3
// CHECK: OpStore
// CHECK: OpReturn
