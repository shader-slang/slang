//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute

// Regression test for shader-slang/slang#11075: HLSL has a native matrix `min`/`max`, and a generic
// `IFloat` `min`/`max` on a matrix lowers to it, as the concrete matrix overload does, not to a
// loop over rows.

T genMin<T : IFloat>(T a, T b)
{
    return min(a, b);
}
T genMax<T : IFloat>(T a, T b)
{
    return max(a, b);
}

RWStructuredBuffer<float3x3> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float3x3 a = outputBuffer[0], b = outputBuffer[1];
    outputBuffer[2] = genMin(a, b);
    outputBuffer[3] = genMax(a, b);
}

// Each specialized function returns HLSL `min`/`max` applied to its matrix parameters. A loop
// over rows would instead index the parameters and return a separate result variable.
// HLSL: float3x3 genMax_{{[0-9]+}}(float3x3 a_{{[0-9]+}}, float3x3 b_{{[0-9]+}})
// HLSL-NOT: }
// HLSL: return max(a_{{[0-9]+}}, b_{{[0-9]+}});
// HLSL: float3x3 genMin_{{[0-9]+}}(float3x3 a_{{[0-9]+}}, float3x3 b_{{[0-9]+}})
// HLSL-NOT: }
// HLSL: return min(a_{{[0-9]+}}, b_{{[0-9]+}});
