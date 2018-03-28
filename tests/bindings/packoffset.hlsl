//TEST:COMPARE_HLSL:-no-mangle -target dxbc-assembly -profile ps_4_0 -entry main

// Let's make sure we generate correct output in cases
// where there are non-trivial `packoffset`s needed

#ifdef __SLANG__
#define R(X) /**/
#else
#define R(X) X
#endif

float4 use(float  val) { return val; };
float4 use(float2 val) { return float4(val,0.0,0.0); };
float4 use(float3 val) { return float4(val,0.0); };
float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s) { return t.Sample(s, 0.0); }

cbuffer CA R(: register(b0))
{
	float4 ca R(: packoffset(c0));
	float3 cb R(: packoffset(c1.x));
	float  cc R(: packoffset(c1.w));
	float2 cd R(: packoffset(c2.x));
	float2 ce R(: packoffset(c2.z));

	Texture2D ta R(: register(t0));
	SamplerState sa R(: register(s0));
}

float4 main() : SV_Target
{
	// Go ahead and use everything in this case:
	return use(ta, sa)
		+  use(ca)
		+  use(cb)
		+  use(cc)
		+  use(cd)
		+  use(ce)
		;
}