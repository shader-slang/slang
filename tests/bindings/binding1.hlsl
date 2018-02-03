//TEST:COMPARE_HLSL:-no-mangle -target dxbc-assembly -profile ps_4_0 -entry main

// We want to make sure that the registers that Spire generates
// are used, even if there are "dead" parameter earlier in the program.
//
// In this case, we declare two each of textures, samplers, and constant
// buffers, and then only use the second one.
// Left to its own devices, the HLSL compiler would usually shift the
// object that was used up to binding slot zero, and eliminate the one
// that wasn't used.
// We expect Spire to generate explicit annotations that stop this from
// happening.

#ifdef __SPIRE__
#define R(X) /**/
#else
#define R(X) X
#endif

float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s) { return t.Sample(s, 0.0); }

Texture2D 		t0 R(: register(t0));
Texture2D 		t1 R(: register(t1));
SamplerState 	s0 R(: register(s0));
SamplerState 	s1 R(: register(s1));

cbuffer C0 R(: register(b0))
{
	float c0;
}

cbuffer C1 R(: register(b1))
{
	float c1;
}

float4 main() : SV_Target
{
	return use(t1,s1) + use(c1);
}