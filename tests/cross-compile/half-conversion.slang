// half-conversion.slang

//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage fragment

cbuffer C
{
	uint4 u;
}

float4 main() : SV_Target
{
	return f16tof32(u);
}
