//TEST:COMPARE_HLSL:-no-mangle -profile ps_5_0 -entry main

// Ensure declaration order in output is correct

#ifndef __SLANG__
#define A A_0
#define x x_0
#define B B_0
#define y y_0
#define C C_0
#define a a_0
#define b b_0
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
