//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -experimental-feature

// Verify that one vector IDotProduct conformance applies when the element type is known only to be
// a builtin numeric scalar. The witness must work for floating-point, signed-integer, and
// unsigned-integer specializations, using a target-native operation where supported and the
// scalar fallback otherwise.

// CHECK: type: int32_t
// CHECK-NEXT: 70
// CHECK-NEXT: -70
// CHECK-NEXT: 60
// CHECK-NEXT: 5

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=16):name=floatInput
//TEST_INPUT:ubuffer(data=[-1 2 -3 4 5 -6 7 -8], stride=16):name=integerInput
//TEST_INPUT:ubuffer(data=[1 2 3 4 8 7 6 5], stride=16):name=unsignedInput
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output

// HLSL-NOT: dot_
// HLSL: dot(
// HLSL-NOT: dot_
// HLSL: dot(
// HLSL-NOT: dot_
// HLSL: dot(
// HLSL-NOT: dot_

// GLSL-LABEL: uint invokeDotProduct_{{[0-9]+}}(
// GLSL: return {{.*}}[0] * {{.*}}[0] + {{.*}}[1] * {{.*}}[1]
// GLSL-LABEL: int invokeDotProduct_{{[0-9]+}}(
// GLSL: return {{.*}}[0] * {{.*}}[0] + {{.*}}[1] * {{.*}}[1]
// GLSL-LABEL: float invokeDotProduct_{{[0-9]+}}(float
// GLSL: return {{.*}} * {{.*}};
// GLSL-LABEL: float invokeDotProduct_{{[0-9]+}}(vec4
// GLSL: return dot(
// METAL-LABEL: uint invokeDotProduct_{{[0-9]+}}(
// METAL: return {{.*}}[int(0)] * {{.*}}[int(0)] + {{.*}}[int(1)] * {{.*}}[int(1)]
// METAL-LABEL: int invokeDotProduct_{{[0-9]+}}(
// METAL: return {{.*}}[int(0)] * {{.*}}[int(0)] + {{.*}}[int(1)] * {{.*}}[int(1)]
// METAL-LABEL: float invokeDotProduct_{{[0-9]+}}(float
// METAL: return {{.*}} * {{.*}};
// METAL-LABEL: float invokeDotProduct_{{[0-9]+}}(float4
// METAL: return dot(

// SPIRV-DAG: OpCapability DotProductInputAll
// SPIRV-DAG: OpExtension "SPV_KHR_integer_dot_product"
// SPIRV-DAG: OpDot
// SPIRV-DAG: OpSDot
// SPIRV-DAG: OpUDot

// WGSL-NOT: dot_
// WGSL: dot(
// WGSL-NOT: dot_
// WGSL: dot(
// WGSL-NOT: dot_
// WGSL: dot(
// WGSL-NOT: dot_

import slang.numerics;

T.Scalar invokeDotProduct<T : IDotProduct>(T left, T right)
{
    return left.dotProductWith(right);
}

T vectorDotProduct<T : IBuiltinScalarNumeric, let N : int>(
    vector<T, N> left,
    vector<T, N> right)
{
    return invokeDotProduct(left, right);
}

StructuredBuffer<float4> floatInput;
StructuredBuffer<int4> integerInput;
StructuredBuffer<uint4> unsignedInput;
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = int(vectorDotProduct(floatInput[0], floatInput[1]));
    output[1] = vectorDotProduct(integerInput[0], integerInput[1]);
    output[2] = int(vectorDotProduct(unsignedInput[0], unsignedInput[1]));
    output[3] = int(vectorDotProduct(
        vector<float, 1>(floatInput[0].x), vector<float, 1>(floatInput[1].x)));
}
