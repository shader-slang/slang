// callable.slang

//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -profile glsl_460+spirv_1_4 -stage callable -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -allow-glsl -stage callable -entry main -target spirv-assembly -emit-spirv-directly

import callable_shared;

Texture2D gAlbedoMap;
SamplerState gSampler;

// CHECK_GLSL: void main(
// CHECK_SPIRV: OpEntryPoint
void main(in out MaterialPayload ioPayload)
{
	ioPayload.albedo = gAlbedoMap.SampleLevel(
		gSampler,
		ioPayload.uv,
		0);
}
