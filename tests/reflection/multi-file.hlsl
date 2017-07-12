//TEST:SIMPLE:-profile ps_4_0 -entry mainFS -target reflection-json tests/reflection/multi-file-extra.hlsl -profile vs_4_0 -entry mainVS

// Here we are testing the case where multiple translation units are provided
// at once, so that we want combined reflection information for the resulting
// program. The other part of this program is in `multi-file-extra.hlsl`.

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
Texture2D sharedT;
SamplerState sharedS;
cbuffer sharedC
{
	float3 sharedCA;
	float  sharedCB;
	float3 sharedCC;
	float2 sharedCD;
}

// Then some parameters specific to this shader
// (these will get placed before the ones in the `extra` file,
// based on how they get named on the command-line)

Texture2D vertexT;
SamplerState vertexS;
cbuffer vertexC
{
	float3 vertexCA;
	float  vertexCB;
	float3 vertexCC;
	float2 vertexCD;
}

// And end with some shared parameters again
Texture2D sharedTV;
Texture2D sharedTF;


float4 mainFS() : SV_Target
{
	// Go ahead and use everything here, just to make sure things got placed correctly
	return use(sharedT, sharedS)
		+  use(sharedCD)
		+  use(vertexT, vertexS)
		+  use(vertexCD)
		+  use(sharedTV, vertexS)
		;
}