//TEST:COMPARE_HLSL: -profile ps_5_0 -entry main -target dxbc-assembly -split-mixed-types

// Make sure we don't crash when desugaring resource in structs,
// when the user also declares multiple variables with a
// single declaration.

#ifdef __SLANG__

cbuffer C
{
	Texture2D t0, t1;
	SamplerState s;
	float2 uv;
};

float4 main() : SV_Target
{
	return t0.Sample(s, uv)
         + t1.Sample(s, uv);
}

#else

cbuffer C : register(b0)
{
	float2 uv;
};

Texture2D SLANG_parameterGroup_C_t0 : register(t0);
Texture2D SLANG_parameterGroup_C_t1 : register(t1);
SamplerState SLANG_parameterGroup_C_s : register(s0);

float4 main() : SV_Target
{
	return SLANG_parameterGroup_C_t0.Sample(SLANG_parameterGroup_C_s, uv)
	     + SLANG_parameterGroup_C_t1.Sample(SLANG_parameterGroup_C_s, uv);
}

#endif


