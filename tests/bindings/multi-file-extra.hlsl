//TEST_IGNORE_FILE:

// Here we are going to test that we can correctly generating bindings when we
// are presented with a program spanning multiple input files (and multiple entry points)

// This file provides the fragment shader, and is only meant to be tested in combination with `multi-file.hlsl`

#ifdef __SPIRE__
#define R(X) /**/
#else
#define R(X) X
#endif

float4 use(float  val) { return val; };
float4 use(float2 val) { return float4(val,0.0,0.0); };
float4 use(float3 val) { return float4(val,0.0); };
float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s) { return t.Sample(s, 0.0); }

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

// Then some parameters specific to this shader.
// These will be placed *after* the ones from the main file,
// and even after the parameters further down in this file
// that end up being shared between the two files.

Texture2D fragmentT R(: register(t4));
SamplerState fragmentS R(: register(s2));
cbuffer fragmentC R(: register(b2))
{
	float3 fragmentCA R(: packoffset(c0));
	float  fragmentCB R(: packoffset(c0.w));
	float3 fragmentCC R(: packoffset(c1));
	float2 fragmentCD R(: packoffset(c2));
}

// And end with some shared parameters again
Texture2D sharedTV R(: register(t2));
Texture2D sharedTF R(: register(t3));


float4 main() : SV_Target
{
	// Go ahead and use everything here, just to make sure things got placed correctly
	return use(sharedT, sharedS)
		+  use(sharedCD)
		+  use(fragmentT, fragmentS)
		+  use(fragmentCD)
		+  use(sharedTF, sharedS)
		;
}