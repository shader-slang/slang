//TEST:COMPARE_HLSL: -split-mixed-types -no-checking -target dxbc-assembly -profile ps_4_0 -entry main

// Confirm that the `-split-mixed-types` flag works.

#ifdef __SLANG__

// HLSL input:
//
// - Uses at least one `import` of Slang code
// - Uses an aggregate type that mixes resource and non-resource types
//

__import type_splitting;

struct Foo
{
	Texture2D 		t;
	SamplerState 	s;
	float2 			u;
};

cbuffer C
{
	Foo foo;
}

float4 main() : SV_Target
{
	return foo.t.Sample(foo.s, foo.u);	
}

#else

// Equivalent raw HLSL:
//
// - Fields of resource type have been stripped from original type definition
// - Fields of resource type get hoisted out of variable declarations
//

struct Foo
{
	float2 u;
};

cbuffer C
{
	Foo foo;
}

Texture2D    SLANG_parameterBlock_C_foo_t;
SamplerState SLANG_parameterBlock_C_foo_s;

float4 main() : SV_Target
{
	return SLANG_parameterBlock_C_foo_t.Sample(SLANG_parameterBlock_C_foo_s, foo.u);	
}

#endif

