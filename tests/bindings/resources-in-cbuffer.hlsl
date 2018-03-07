//TEST:COMPARE_HLSL:-no-mangle -target dxbc-assembly -profile ps_4_0 -entry main

// Confirm that resources inside constant buffers get correct locations,
// including the case where there are *multiple* constant buffers
// with reosurces.

#ifdef __SLANG__
#define R(X) /**/
#else
#define R(X) X

#define CA _SV023SLANG_parameterGroup_CAL0
#define caa _SV023SLANG_ParameterGroup_CA3caa
#define cab _SV023SLANG_ParameterGroup_CA3cab
#define cac _SV023SLANG_ParameterGroup_CA3cac
#define cad _SV023SLANG_ParameterGroup_CA3cad
#define cae _SV023SLANG_ParameterGroup_CA3cae
#define ta 	_SV023SLANG_parameterGroup_CAL1
#define sa 	_SV023SLANG_parameterGroup_CAL2

#define CB _SV023SLANG_parameterGroup_CBL0
#define cba _SV023SLANG_ParameterGroup_CB3cba
#define cbb _SV023SLANG_ParameterGroup_CB3cbb
#define cbc _SV023SLANG_ParameterGroup_CB3cbc
#define cbd _SV023SLANG_ParameterGroup_CB3cbd
#define cbe _SV023SLANG_ParameterGroup_CB3cbe
#define tbx	_SV023SLANG_parameterGroup_CBL1
#define tby	_SV023SLANG_parameterGroup_CBL2
#define sb 	_SV023SLANG_parameterGroup_CBL3

#define CC _SV023SLANG_parameterGroup_CCL0
#define cca _SV023SLANG_ParameterGroup_CC3cca
#define ccb _SV023SLANG_ParameterGroup_CC3ccb
#define ccc _SV023SLANG_ParameterGroup_CC3ccc
#define ccd _SV023SLANG_ParameterGroup_CC3ccd
#define cce _SV023SLANG_ParameterGroup_CC3cce
#define tc 	_SV023SLANG_parameterGroup_CCL1
#define scx	_SV023SLANG_parameterGroup_CCL2
#define scy	_SV023SLANG_parameterGroup_CCL3

#endif

float4 use(float  val) { return val; };
float4 use(float2 val) { return float4(val,0.0,0.0); };
float4 use(float3 val) { return float4(val,0.0); };
float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s) { return t.Sample(s, 0.0); }

cbuffer CA R(: register(b0))
{
	float4 caa R(: packoffset(c0));
	float3 cab R(: packoffset(c1.x));
	float  cac R(: packoffset(c1.w));
	float2 cad R(: packoffset(c2.x));
	float2 cae R(: packoffset(c2.z));

	Texture2D ta R(: register(t0));
	SamplerState sa R(: register(s0));
}

cbuffer CB R(: register(b1))
{
	float4 cba R(: packoffset(c0));
	float3 cbb R(: packoffset(c1.x));
	float  cbc R(: packoffset(c1.w));
	float2 cbd R(: packoffset(c2.x));
	float2 cbe R(: packoffset(c2.z));

	Texture2D tbx R(: register(t1));
	Texture2D tby R(: register(t2));
	SamplerState sb R(: register(s1));
}

cbuffer CC R(: register(b2))
{
	float4 cca R(: packoffset(c0));
	float3 ccb R(: packoffset(c1.x));
	float  ccc R(: packoffset(c1.w));
	float2 ccd R(: packoffset(c2.x));
	float2 cce R(: packoffset(c2.z));

	Texture2D tc R(: register(t3));
	SamplerState scx R(: register(s2));
	SamplerState scy R(: register(s3));
}

float4 main() : SV_TARGET
{
	// Go ahead and use everything in this case:
	return use(ta,  sa)
		+  use(tbx, sb)
		+  use(tby, scx)
		+  use(tc,  scy)
		+  use(cae)
		+  use(cbe)
		+  use(cce)
		;
}