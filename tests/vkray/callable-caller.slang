// callable-caller.slang

//TEST:CROSS_COMPILE: -profile sm_6_3 -stage raygeneration -entry main -target spirv-assembly

import callable_shared;

cbuffer C
{
	uint shaderIndex;
};

RWTexture2D<float4> gImage;

void main()
{
	MaterialPayload payload;
	payload.albedo = 0;
	payload.uv = float2(DispatchRaysIndex().xy) / float2(DispatchRaysDimensions().xy);

	CallShader(shaderIndex, payload);

	gImage[DispatchRaysIndex().xy] = payload.albedo;
}
