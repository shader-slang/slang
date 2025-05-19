//TEST:SIMPLE(filecheck=CHECK): -target spirv

[[SpecializationConstant]] const float NEAR_PLANE = 0.1f;
[[SpecializationConstant]] const float FAR_PLANE = 256.0f;

// `OpSpecConstantOp` can only contain integer operations when targeting Vulkan SPIRV, not floating-point operations.
// This test ensures that floating-point operations that strictly contain specialization constant variables are not declared with `OpSpecContantOp`.

// CHECK-COUNT-2: OpSpecConstant

float linearDepth(float depth)
{
    float z = depth;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return linearDepth(1.0);
}
