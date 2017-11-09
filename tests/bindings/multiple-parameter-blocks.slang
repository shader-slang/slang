//TEST:COMPARE_HLSL:-no-mangle -use-ir -target dxbc-assembly -profile ps_5_1 -entry main -parameter-blocks-use-register-spaces

// Confirm that Slang `ParameterBlock<T>` generates
// parameter bindings like we expect.


float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s) { return t.Sample(s, 0.0); }

#ifdef __SLANG__

struct S
{
	Texture2D 		t;
	Texture2D 		ta[4];
	SamplerState 	s;	
};

ParameterBlock<S> p;
ParameterBlock<S> p1;

float4 main(float v : V) : SV_Target
{
	return use(p.t, p.s)
		+ use(p.ta[int(v)], p.s)
		+ use(p1.t, p1.s)
		+ use(p1.ta[int(v)], p1.s);
}

#else

Texture2D _S1 : register(t0, space0);
Texture2D _S2[4] : register(t1, space0);
SamplerState _S3 : register(s0, space0);

Texture2D _S12 : register(t0, space1);
Texture2D _S13[4] : register(t1, space1);
SamplerState _S14 : register(s0, space1);

float4 main(float v : V) : SV_Target
{
	return use(_S1, _S3)
		+ use(_S2[int(v)], _S3)
		+ use(_S12, _S14)
		+ use(_S13[int(v)], _S14);
}

#endif
