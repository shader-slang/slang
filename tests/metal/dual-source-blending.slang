//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage fragment

// Test that dual-source blending outputs using vk::location and
// vk::index produce correct Metal color/index attributes instead
// of separate color attachments.

// METAL: color(0)
// METAL: color(0), index(1)

struct FragmentOutput
{
	[[vk::location(0)]]
	float4 a : SV_Target0;

	[[vk::location(0), vk::index(1)]]
	float4 b : SV_Target0;
}

[shader("fragment")]
FragmentOutput main(float4 v : V)
{
	FragmentOutput f = {};
	f.a = v;
	f.b = v;
	return f;
}
