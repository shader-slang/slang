//TEST:COMPARE_HLSL:-no-mangle -profile ps_4_0 -entry main

// Ensure that matrix-times-scalar works

#ifndef __SLANG__
#define C _SV022SLANG_parameterGroup_C
#define a _SV022SLANG_ParameterGroup_C1a
#define b _SV022SLANG_ParameterGroup_C1b
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
