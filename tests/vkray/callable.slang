// callable.slang

//TEST:CROSS_COMPILE: -profile sm_6_3 -stage callable -entry main -target spirv-assembly

import callable_shared;

Texture2D gAlbedoMap;
SamplerState gSampler;

void main(in out MaterialPayload ioPayload)
{
	ioPayload.albedo = gAlbedoMap.SampleLevel(
		gSampler,
		ioPayload.uv,
		0);
}
