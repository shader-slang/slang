//TEST:CROSS_COMPILE: -profile ps_5_0 -entry main -target spirv-assembly

// Ensure that we can lower to `imageStore` correctly.

RWTexture2D<float4> t;

void writeColor(float3 v)
{
    t[uint2(0,0)].xyz += v;
}

float4 main() : SV_Target
{
	writeColor(float3(1.0));
	return float4(0);
}
