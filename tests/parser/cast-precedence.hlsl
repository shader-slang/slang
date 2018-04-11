//TEST:COMPARE_HLSL: -profile vs_5_0

// Confirm that type-cast expressions parse with
// the appropriate precedence.

#ifndef __SLANG__
#define C _SV022SLANG_parameterGroup_C
#define a _SV022SLANG_ParameterGroup_C1a
#define b _SV022SLANG_ParameterGroup_C1b
#define SV_Position SV_POSITION
#endif

cbuffer C : register(b0)
{
	float a;
	float b;
};

float4 main() : SV_Position
{
	return (uint) a / b;
}
