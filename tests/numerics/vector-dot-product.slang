// Verify that one vector IDotProduct conformance applies when the element type is known only to be
// a builtin numeric scalar. The witness must work for both floating-point and integer
// specializations and lower to the corresponding target-native dot operation.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -experimental-feature

// HLSL: dot(
// HLSL: dot(

// SPIRV: OpDot
// SPIRV: OpSDot

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
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = vectorDotProduct(floatInput[0], floatInput[1]);
    output[1] = float(vectorDotProduct(integerInput[0], integerInput[1]));
}
