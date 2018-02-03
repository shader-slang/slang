//TEST:COMPARE_HLSL:-no-mangle -profile ps_4_0 -entry main

// Ensure that matrix-times-scalar works

float4x4 doIt(float4x4 a, float b)
{
	return a * b;
}

cbuffer C
{
	float4x4 a;
	float b;	
};

float4 main() : SV_Target
{
	return doIt(a, b)[0];
}
