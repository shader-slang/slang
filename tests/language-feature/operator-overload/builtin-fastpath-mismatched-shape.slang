//TEST:SIMPLE(filecheck=CHECK): -entry main -stage compute -target hlsl

// Mismatched vector extents are not broadcast-compatible: `getBuiltinArithmeticCommonType`
// returns null, so the fast path bails and overload resolution reports "no overload" -- rather
// than the fast path silently producing a componentwise op on mismatched shapes.

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    float3 a = float3(1);
    float4 b = float4(1);
    // CHECK: no overload for '+'
    outputBuffer[0] = (a + b).x;
}
