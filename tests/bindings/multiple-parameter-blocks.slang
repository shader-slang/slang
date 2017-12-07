//TEST:COMPARE_HLSL:-use-ir -target dxbc-assembly -profile ps_5_1 -entry main -parameter-blocks-use-register-spaces

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

Texture2D _SV01pL0 : register(t0, space0);
Texture2D _SV01pL1[4] : register(t1, space0);
SamplerState _SV01pL2 : register(s0, space0);

Texture2D _SV02p1L0 : register(t0, space1);
Texture2D _SV02p1L1[4] : register(t1, space1);
SamplerState _SV02p1L2 : register(s0, space1);

float4 main(float v : V) : SV_Target
{
	return use(_SV01pL0,          _SV01pL2)
		 + use(_SV01pL1[int(v)],  _SV01pL2)
		 + use(_SV02p1L0,         _SV02p1L2)
		 + use(_SV02p1L1[int(v)], _SV02p1L2);
}

#endif
