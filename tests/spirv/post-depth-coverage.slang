// post-depth-coverage.slang
//
// Tests the `[postdepthcoverage]` fragment entry-point attribute (shader-slang/slang#12999).
// On SPIR-V it must emit the `SampleMaskPostDepthCoverage` capability, declare the
// `SPV_KHR_post_depth_coverage` extension, and enable the `PostDepthCoverage` execution mode so the
// input `SV_Coverage` reports only the samples that survived the early depth/stencil test.
//
// PostDepthCoverage is only valid alongside EarlyFragmentTests (Vulkan requires the pairing, and
// GL_ARB_post_depth_coverage defines post_depth_coverage to imply early_fragment_tests), so the
// compiler also emits EarlyFragmentTests. This is checked both when the attribute is paired with
// `[earlydepthstencil]` (mainPaired) and when it is used on its own (mainAlone) — the two entry
// points must produce identical execution modes.
//
// The opt-in / absence case (no attribute => none of these emitted) is covered by the companion
// test post-depth-coverage-negative.slang.

//TEST:SIMPLE(filecheck=SPV): -target spirv
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry mainAlone -stage fragment
//TEST:SIMPLE(filecheck=GLSLPAIRED): -target glsl -entry mainPaired -stage fragment
//TEST:SIMPLE(filecheck=DEDUP): -target spirv -entry mainPaired -stage fragment

// The capability and extension are module-level; each entry point carries both execution modes.
// SPV-DAG: OpCapability SampleMaskPostDepthCoverage
// SPV-DAG: OpExtension "SPV_KHR_post_depth_coverage"
// SPV-DAG: OpExecutionMode %mainPaired EarlyFragmentTests
// SPV-DAG: OpExecutionMode %mainPaired PostDepthCoverage
// SPV-DAG: OpExecutionMode %mainAlone EarlyFragmentTests
// SPV-DAG: OpExecutionMode %mainAlone PostDepthCoverage

// mainAlone carries only [postdepthcoverage]: GLSL requires the extension and emits the qualifier;
// early_fragment_tests is implied by the extension, so it is not redeclared.
// GLSL-DAG: GL_ARB_post_depth_coverage
// GLSL-DAG: layout(post_depth_coverage) in;

// mainPaired carries both [earlydepthstencil] and [postdepthcoverage]: the GLSL path has no dedup,
// so it emits both input layout qualifiers (declaring the ARB-implied early_fragment_tests
// explicitly is valid GLSL).
// GLSLPAIRED-DAG: GL_ARB_post_depth_coverage
// GLSLPAIRED-DAG: layout(early_fragment_tests) in;
// GLSLPAIRED-DAG: layout(post_depth_coverage) in;

// Dedup guard: with both [earlydepthstencil] and [postdepthcoverage] present, EarlyFragmentTests
// must be emitted exactly once (the SPIR-V execution-mode funnel dedups). COUNT-1 + NOT asserts the
// count, so a regression that double-emitted it would fail here rather than pass a presence check.
// DEDUP-COUNT-1: OpExecutionMode %mainPaired EarlyFragmentTests
// DEDUP-NOT: OpExecutionMode %mainPaired EarlyFragmentTests

[earlydepthstencil]
[postdepthcoverage]
[shader("fragment")]
float4 mainPaired(uint coverage : SV_Coverage) : SV_Target
{
    return float4((float)coverage, 0, 0, 1);
}

[postdepthcoverage]
[shader("fragment")]
float4 mainAlone(uint coverage : SV_Coverage) : SV_Target
{
    return float4((float)coverage, 0, 0, 1);
}
