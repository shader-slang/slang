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

float4 main(float v : V) : SV_Target
{
	return use(p.ta[int(v)], p.s)
		+ use(p.t, p.s);
}

#else

Texture2D t : register(t0, space0);
Texture2D ta[4] : register(t1, space0);
SamplerState s : register(s0, space0);

float4 main(float v : V) : SV_Target
{
	return use(ta[int(v)], s)
		+ use(t, s);
}

#endif
