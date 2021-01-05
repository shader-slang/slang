// callable.slang

//TEST:CROSS_COMPILE: -profile glsl_460+spirv_1_4 -stage callable -entry main -target spirv-assembly

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
