//TEST_IGNORE_FILE:

// Imported code used by `reflect-imported-code.hlsl`

float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s) { return t.Sample(s, 0.0); }

Texture2D 		t_i;
SamplerState 	s_i;

cbuffer C_i
{
	float c_i;
}
