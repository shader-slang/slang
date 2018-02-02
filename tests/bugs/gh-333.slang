//TEST:COMPARE_HLSL:-no-mangle -profile ps_5_0 -entry main

// Ensure declaration order in output is correct

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

float4 main() : SV_Target
{
	return a.x;
}
