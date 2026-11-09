//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -experimental-feature -o differentiable-module.hlsl

import slang.numerics.differentiable;

RWStructuredBuffer<float4> outputBuffer;

[Differentiable]
T differentiableSine<T : IDifferentiableTrigonometricFunctions>(T value)
{
    return sin(value);
}

[Differentiable]
float scalarSine(float value)
{
    return differentiableSine(value);
}

[Differentiable]
float3 vectorSine(float3 value)
{
    return differentiableSine(value);
}

T addToSelf<T : IAdditive>(T value)
{
    return value + value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let scalarResult = fwd_diff(scalarSine)(diffPair(1.0, 1.0));
    let vectorResult = fwd_diff(vectorSine)(diffPair(float3(1.0), float3(1.0)));
    var scalarReverse = diffPair(1.0, 0.0);
    bwd_diff(scalarSine)(scalarReverse, 1.0);
    var vectorReverse = diffPair(float3(1.0), float3(0.0));
    bwd_diff(vectorSine)(vectorReverse, float3(1.0));
    float3 baseModuleResult = addToSelf(float3(1.0));
    outputBuffer[0] =
        float4(
            scalarResult.d + scalarReverse.d,
            vectorResult.d + vectorReverse.d + baseModuleResult);
}
