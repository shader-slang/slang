//TEST:COMPARE_HLSL: -profile vs_5_0

// Confirm that type-cast expressions parse with
// the appropriate precedence.

cbuffer C : register(b0)
{
	float a;
	float b;
};

float4 main() : SV_Position
{
	return (uint) a / b;
}
