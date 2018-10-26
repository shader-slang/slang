//TEST:CROSS_COMPILE: -profile ps_5_0 -entry main -target spirv-assembly

// Ensure that we can lower to `imageAtomicAdd` correctly.

RWTexture2D<uint> t;

float4 main() : SV_Target
{
	uint u;
	InterlockedAdd(t[uint2(0)], 1, u);
	return u;
}
