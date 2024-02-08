// callable-caller.slang

//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -profile glsl_460 -capability GL_NV_ray_tracing -stage raygeneration -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -allow-glsl -capability GL_NV_ray_tracing -stage raygeneration -entry main -target spirv-assembly -emit-spirv-directly

import callable_shared;

cbuffer C
{
	uint shaderIndex;
};

RWTexture2D<float4> gImage;

// CHECK_GLSL: void main(
// CHECK_SPIRV: OpEntryPoint
void main()
{
	MaterialPayload payload;
	payload.albedo = 0;
	payload.uv = float2(DispatchRaysIndex().xy) / float2(DispatchRaysDimensions().xy);

	CallShader(shaderIndex, payload);

	gImage[DispatchRaysIndex().xy] = payload.albedo;
}
