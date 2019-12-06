// multi-file-shared.slang
//TEST_IGNORE_FILE:

#include "multi-file-defines.h"

float4 use(float  val) { return val; };
float4 use(float2 val) { return float4(val,0.0,0.0); };
float4 use(float3 val) { return float4(val,0.0); };
float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s) { return t.SampleLevel(s, 0.0, 0.0); }

Texture2D sharedT R(: register(t2));
SamplerState sharedS R(: register(s2));

BEGIN_CBUFFER(sharedC)
{
	float3 sharedCA;
	float  sharedCB;
	float3 sharedCC;
	float2 sharedCD;
}
END_CBUFFER(sharedC, register(b2))

Texture2D sharedTV R(: register(t3));
Texture2D sharedTF R(: register(t4));
