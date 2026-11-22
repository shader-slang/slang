// post-depth-coverage-negative.slang
//
// Negative companion to post-depth-coverage.slang (shader-slang/slang#12999).
//
// This fragment entry point carries neither `[postdepthcoverage]` nor `[earlydepthstencil]`, so it
// must emit none of the post-depth-coverage capability/extension/execution mode and no
// `EarlyFragmentTests` (which would otherwise come from either attribute). This guards that the
// emission is opt-in and is not produced unconditionally for every fragment entry point — a
// regression that emitted `EarlyFragmentTests` unconditionally would silently change depth-test
// ordering for every Vulkan fragment shader. Mirrors the opt-in guard in
// tests/spirv/shader-64bit-indexing-negative.slang.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK-NOT: SampleMaskPostDepthCoverage
// CHECK-NOT: SPV_KHR_post_depth_coverage
// CHECK-NOT: PostDepthCoverage
// CHECK-NOT: EarlyFragmentTests

[shader("fragment")]
float4 main(uint coverage : SV_Coverage) : SV_Target
{
    return float4((float)coverage, 0, 0, 1);
}
