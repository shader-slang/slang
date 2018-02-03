//TEST:COMPARE_HLSL:-no-mangle -target dxbc-assembly -profile ps_4_0 -entry main

// Let's first confirm that Spire can reproduce what the
// HLSL compiler would already do in the simple case (when
// all shader parameters are actually used).

#ifdef __SPIRE__
#define R(X) /**/
#else
#define R(X) X
#endif

float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s) { return t.Sample(s, 0.0); }

Texture2D 		t R(: register(t0));
SamplerState 	s R(: register(s0));

cbuffer C R(: register(b0))
{
	float c;
}

float4 main() : SV_Target
{
	return use(t,s) + use(c);
}