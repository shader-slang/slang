//TEST:COMPARE_HLSL: -profile ps_5_0 -entry main -target dxbc-assembly -split-mixed-types
// Make sure we don't crash when desugaring resources
// in structs when a `cbuffer` only contains resources.

#ifdef __SLANG__

cbuffer C
{
	Texture2D t;
	SamplerState s;
};

float4 main(float2 uv: UV) : SV_Target
{
	return t.Sample(s, uv);
}

#else

Texture2D SLANG_parameterGroup_C_t : register(t0);
SamplerState SLANG_parameterGroup_C_s : register(s0);

float4 main(float2 uv: UV) : SV_Target
{
	return SLANG_parameterGroup_C_t.Sample(SLANG_parameterGroup_C_s, uv);
}

#endif


