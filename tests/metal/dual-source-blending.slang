//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage fragment
//TEST:SIMPLE(filecheck=METAL_MRT):-target metal -entry mrtMain -stage fragment

// Test that dual-source blending outputs using vk::location and
// vk::index produce correct Metal color/index attributes instead
// of separate color attachments.

// METAL: [[color(0)]]
// METAL: [[color(0), index(1)]]
// METAL-NOT: [[color(1)]]

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

// Regression: normal MRT without vk::index must still produce
// separate color attachments, not dual-source attributes.

struct MRTOutput
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
}

// METAL_MRT: [[color(0)]]
// METAL_MRT: [[color(1)]]
// METAL_MRT-NOT: index(

[shader("fragment")]
MRTOutput mrtMain(float4 v : V)
{
	MRTOutput o = {};
	o.color0 = v;
	o.color1 = v;
	return o;
}
