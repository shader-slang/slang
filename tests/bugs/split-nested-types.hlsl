//TEST:COMPARE_HLSL:-no-mangle -profile ps_5_0

#ifdef __SLANG__
import split_nested_types;
#else

#define A A_0
#define x x_0

#define B B_0
#define y y_0

#define M M_0
#define a a_0
#define b b_0

#define C C_0
#define m m_0

struct A { int x; };

struct B { float y; };

struct CC { Texture2D t; SamplerState s; };

struct M
{
	A a;
	B b;
};

#endif

cbuffer C
{
	M m;
}

float4 main() : SV_TARGET
{
	return m.b.y;
}
