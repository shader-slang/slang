//TEST:COMPARE_HLSL:-no-mangle -profile ps_4_0 -entry main

// Ensure that matrix-times-scalar works

#ifndef __SLANG__
#define C C_0
#define a a_0
#define b b_0
#endif

float4x4 doIt(float4x4 a, float b)
{
	return a * b;
}

cbuffer C
{
	float4x4 a;
	float b;	
};

float4 main() : SV_TARGET
{
	return doIt(a, b)[0];
}
