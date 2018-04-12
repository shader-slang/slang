//TEST:COMPARE_HLSL:-no-mangle -profile ps_5_0 -entry main

// Ensure declaration order in output is correct

#ifndef __SLANG__
#define A _ST01A
#define x _SV01A1x
#define B _ST01B
#define y _SV01B1y
#define C _SV022SLANG_parameterGroup_CL0
#define a _SV022SLANG_ParameterGroup_C1a
#define b _SV022SLANG_ParameterGroup_C1b
#endif

struct A
{
	float x;
};

struct B
{
	float y;
	Texture2D t;
};

cbuffer C
{
	A a;
	B b;
};

float4 main() : SV_TARGET
{
	return a.x;
}
