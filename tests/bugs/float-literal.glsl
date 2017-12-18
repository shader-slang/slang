//TEST:COMPARE_GLSL:-profile glsl_fragment_450 -no-checking
#version 450

layout(set = 0, binding = 1) cbuffer PerFrameCB : register(b0)
{
	float4x4 gvpTransform;
	float3 gFontColor[2];
};

float4 transform(float2 posS)
{
	return mul(float4(posS, 0.5f, 1), gvpTransform);
}

void main(
	float2 posS : POSITION,
	inout float2 texC : TEXCOORD,
	out float4 posSV : SV_POSITION)
{
	posSV = transform(posS);
}
