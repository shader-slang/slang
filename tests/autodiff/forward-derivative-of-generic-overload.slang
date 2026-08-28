//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile cs_6_0 -entry computeMain -line-directive-mode none

__generic<T : __BuiltinFloatingPointType, let N : int>
[Differentiable]
vector<T, N> testFunction(vector<T, N> value)
{
    return value * value;
}

[Differentiable]
T testFunction<T : IFloat>(T value)
{
    return value * value;
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[ForwardDerivativeOf(testFunction)]
DifferentialPair<vector<T, N>> testFunctionForward(
    DifferentialPair<vector<T, N>> value)
{
    // Use an intentionally incorrect factor so the emitted code proves that this custom
    // derivative was selected instead of an automatically generated derivative.
    return diffPair(
        testFunction(value.p),
        T(3.0) * value.p * value.d);
}

[ForwardDifferentiable]
float3 callTestFunction(float3 value)
{
    return testFunction(value);
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let result = fwd_diff(callTestFunction)(
        diffPair(float3(2.0), float3(1.0)));
    outputBuffer[0] = result.d.x;
}

// CHECK: testFunctionForward
// CHECK: 3.0f
// CHECK: testFunctionForward
