//dual-source-blending.slang

// Test handling of `vk::location` and
// `vk::index` attributes to compile
// for dual-source color blending.

//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage fragment
//TEST_DISABLED:SIMPLE:-target glsl

struct FragmentOutput
{
	[[vk::location(0)]]
	float4 a : SV_Target0;

	[[vk::location(0), vk::index(1)]]
	float4 b : SV_Target1;
}

[shader("fragment")]
FragmentOutput main(float4 v : V)
{
	FragmentOutput f = {};
	f.a = v;
	f.b = v;
	return f;	
}
