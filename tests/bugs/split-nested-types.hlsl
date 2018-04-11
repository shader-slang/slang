//TEST:COMPARE_HLSL:-no-mangle -profile ps_5_0 -target dxbc-assembly

#ifdef __SLANG__
import split_nested_types;
#else

#define A _ST01A
#define x _SV01A1x

#define B _ST01B
#define y _SV01B1y

#define M _ST01M
#define a _SV01M1a
#define b _SV01M1b

#define C _SV022SLANG_parameterGroup_CL0
#define m _SV022SLANG_ParameterGroup_C1m

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
