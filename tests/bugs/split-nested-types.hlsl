// array-size-static-const.hlsl
//TEST:COMPARE_HLSL: -profile ps_5_0 -target dxbc-assembly

#ifdef __SLANG__
import split_nested_types;
#else

struct A { int x; };

struct B { float y; };

struct C { Texture2D t; SamplerState s; };

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

float4 main() : SV_target
{
	return m.b.y;
}
