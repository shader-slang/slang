//TEST:CROSS_COMPILE: -profile ps_5_0 -entry main -target spirv-assembly

// Ensure that we add the `GL_EXT_nonuniform_qualifier` extension for any code that uses unbounded-size arrays of resources.

Texture2D t[];
SamplerState s;

cbuffer C
{
	float2 uv;
	uint index;
}

float4 main() : SV_Target
{
	return t[index].Sample(s, uv);
}
