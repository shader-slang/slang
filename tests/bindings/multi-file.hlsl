//TEST:COMPARE_HLSL: -target dxbc-assembly -profile vs_4_0 -entry main Tests/bindings/multi-file-extra.hlsl -profile ps_4_0 -entry main

// Here we are going to test that we can correctly generating bindings when we
// are presented with a program spanning multiple input files (and multiple entry points)

// This file provides the vertex shader, while the fragment shader resides in
// the file `multi-file-extra.hlsl`

#ifdef __SPIRE__
#define R(X) /**/
#else
#define R(X) X
#endif

float4 use(float  val) { return val; };
float4 use(float2 val) { return float4(val,0.0,0.0); };
float4 use(float3 val) { return float4(val,0.0); };
float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s)
{
	// This is the vertex shader, so we can't do implicit-gradient sampling
	return t.SampleGrad(s, 0.0, 0.0, 0.0);
}

// Start with some parameters that will appear in both shaders
Texture2D sharedT R(: register(t0));
SamplerState sharedS R(: register(s0));
cbuffer sharedC R(: register(b0))
{
	float3 sharedCA R(: packoffset(c0));
	float  sharedCB R(: packoffset(c0.w));
	float3 sharedCC R(: packoffset(c1));
	float2 sharedCD R(: packoffset(c2));
}

// Then some parameters specific to this shader
// (these will get placed before the ones in the `extra` file,
// based on how they get named on the command-line)

Texture2D vertexT R(: register(t1));
SamplerState vertexS R(: register(s1));
cbuffer vertexC R(: register(b1))
{
	float3 vertexCA R(: packoffset(c0));
	float  vertexCB R(: packoffset(c0.w));
	float3 vertexCC R(: packoffset(c1));
	float2 vertexCD R(: packoffset(c2));
}

// And end with some shared parameters again
Texture2D sharedTV R(: register(t2));
Texture2D sharedTF R(: register(t3));


float4 main() : SV_Position
{
	// Go ahead and use everything here, just to make sure things got placed correctly
	return use(sharedT, sharedS)
		+  use(sharedCD)
		+  use(vertexT, vertexS)
		+  use(vertexCD)
		+  use(sharedTV, vertexS)
		;
}