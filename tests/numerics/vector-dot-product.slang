//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -experimental-feature

// Verify that one vector IDotProduct conformance applies when the element type is known only to be
// a builtin numeric scalar. The witness must work for floating-point, signed-integer, and
// unsigned-integer specializations and lower to the corresponding target-native dot operation.

// CHECK: type: int32_t
// CHECK-NEXT: 70
// CHECK-NEXT: -70
// CHECK-NEXT: 60

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=16):name=floatInput
//TEST_INPUT:ubuffer(data=[-1 2 -3 4 5 -6 7 -8], stride=16):name=integerInput
//TEST_INPUT:ubuffer(data=[1 2 3 4 8 7 6 5], stride=16):name=unsignedInput
//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=output

// HLSL: dot(
// HLSL: dot(
// HLSL: dot(

// SPIRV: OpDot
// SPIRV: OpSDot
// SPIRV: OpUDot

// WGSL: dot(
// WGSL: dot(
// WGSL: dot(

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
}
