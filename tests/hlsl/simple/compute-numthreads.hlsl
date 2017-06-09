//TEST:COMPARE_HLSL: -no-checking -target dxbc-assembly -profile cs_5_0 -entry main

// Confirm that we properly pass along the `numthreads` attribute on an entry point.

RWStructuredBuffer<float> b;

[numthreads(32,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
	b[tid.x] = b[tid.x + 1] + 1.0f;
}