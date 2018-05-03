//TEST:COMPARE_HLSL: -profile vs_5_0

// Confirm that type-cast expressions parse with
// the appropriate precedence.

#ifndef __SLANG__
#define C C_0
#define a a_0
#define b b_0
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
