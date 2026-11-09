// Verify that the extensions from slang.functional and slang.numerics coexist in either import
// order.

//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -experimental-feature -DFUNCTIONAL_FIRST -o functional-first.hlsl
//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -experimental-feature -o numerics-first.hlsl

#ifdef FUNCTIONAL_FIRST
import slang.functional;
import slang.numerics;
#else
import slang.numerics;
import slang.functional;
#endif

RWStructuredBuffer<float4> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float3 generated = float3((int index) => float(index + 1));
    float3 mapped = generated.map((float value) => value + 1.0);
    outputBuffer[0] = float4(sin(mapped), 1.0);
}
