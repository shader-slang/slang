//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry rayGenMain -stage raygeneration -emit-spirv-directly

// Test for issue #8902: unbounded arrays of RaytracingAccelerationStructure
// should not emit ArrayStride decoration since acceleration structures are opaque types.

// CHECK: OpCapability RuntimeDescriptorArray
// CHECK: OpCapability RayTracingKHR
// CHECK: OpTypeAccelerationStructureKHR
// CHECK: OpTypeRuntimeArray
// CHECK-NOT: OpDecorate %{{.*}} ArrayStride
// CHECK: OpVariable %{{.*}} UniformConstant

[[vk::binding(0, 0)]] RaytracingAccelerationStructure kTLAS[];

struct RayPayload {
  float3 color;
};

[shader("raygeneration")]
void rayGenMain() {
  RayDesc ray;
  ray.Origin = float3(0, 0, 0);
  ray.Direction = float3(0, 0, 1);
  ray.TMin = 0.1;
  ray.TMax = 100.0;

  RayPayload payload = { float3(0.0, 0.0, 0.0) };

  // Access the unbounded array - should produce valid SPIR-V without ArrayStride
  TraceRay(
    kTLAS[0],
    RAY_FLAG_FORCE_OPAQUE,
    0xff,
    0,
    0,
    0,
    ray,
    payload
  );
}

[shader("miss")]
void missMain(inout RayPayload payload) {
  payload.color = float3(0.0, 0.0, 0.0);
}