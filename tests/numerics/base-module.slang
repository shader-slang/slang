//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -experimental-feature -o base-module.spv

import slang.numerics;

// HLSL-NOT: IAdditive
// HLSL-NOT: ITrigonometricFunctions
// HLSL: sin(

RWStructuredBuffer<float4> outputBuffer;

struct CustomValue : IScalarAdditive, ITrigonometricFunctions
{
    float value;

    typealias Scalar = CustomValue;
    typealias Mask = bool;

    static This fromScalar(Scalar value) { return value; }
    static This zero() { return {0.0}; }
    This plus(This other) { return {value + other.value}; }
    This minus(This other) { return {value - other.value}; }
    This sin() { return {::sin(value)}; }
    This cos() { return {::cos(value)}; }
    This tan() { return {::tan(value)}; }
    void sincos(out This sineResult, out This cosineResult)
    {
        sineResult = {::sin(value)};
        cosineResult = {::cos(value)};
    }
}

T wave<T : IAdditive & ITrigonometricFunctions>(T value)
{
    return sin(value + value);
}

T legacyDouble<T : IFloat>(T value)
{
    return value + value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float scalarResult = wave(1.0);
    float3 vectorResult = wave(float3(1.0, 2.0, 3.0));
    CustomValue customResult = wave(CustomValue(1.0));
    outputBuffer[0] = float4(scalarResult, vectorResult) + customResult.value;
    outputBuffer[1] = float4(
        legacyDouble(float3(1.0)),
        ::sin(1.0 + 1.0));
}
