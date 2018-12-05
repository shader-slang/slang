//TEST:REFLECTION:-stage fragment -target glsl
//TEST:REFLECTION:-stage fragment -target hlsl -profile sm_5_0
//TEST:REFLECTION:-stage fragment -target hlsl -profile sm_5_1

// Confirm that we do parameter binding correctly
// when we have both a parameter block *and* user-defined
// resource parameters that both need automatic
// binding allocation.

struct Helper
{
	Texture2D t;
	SamplerState s;	
};

ParameterBlock<Helper> a;

Texture2D b;

float4 main() : SV_target
{ return 0.0; }
