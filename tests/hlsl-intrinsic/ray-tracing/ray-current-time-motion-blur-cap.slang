// Regression test for https://github.com/shader-slang/slang/issues/11841
//
// RayCurrentTime() lowers to the SPIR-V `CurrentRayTimeNV` BuiltIn, which requires the
// `RayTracingMotionBlurNV` capability and the `SPV_NV_ray_tracing_motion_blur` extension.
// Before the fix these were never declared in the intrinsic's `spirv_asm` block, so the
// emitted module failed spirv-val / vkCreateShaderModule with
// VUID-VkShaderModuleCreateInfo-pCode-08737. Assert both are now emitted.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage closesthit -entry main
// Also compile to a binary SPIR-V module so CI's SLANG_RUN_SPIRV_VALIDATION=1 runs
// spirv-val against the real module — the precise validation #11841 reported failing.
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage closesthit -entry main

// CHECK-DAG: OpCapability RayTracingMotionBlurNV
// CHECK-DAG: OpExtension "SPV_NV_ray_tracing_motion_blur"
// CHECK-DAG: OpDecorate %{{.*}} BuiltIn CurrentRayTimeNV

struct Payload
{
    float4 color;
};

[shader("closesthit")]
void main(inout Payload payload, BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = float4(RayCurrentTime(), 0, 0, 1);
}
