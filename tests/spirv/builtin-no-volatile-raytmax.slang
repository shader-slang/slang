// builtin-no-volatile-raytmax.slang
//
// Regression test for shader-slang/slang#10528. `RayTCurrent()` lowers
// to `OpLoad builtin(RayTmaxKHR)`. The Vulkan SPIR-V env spec mandates
// `Volatile` semantics on `RayTmaxKHR` only in the Intersection stage;
// in AnyHit, ClosestHit, and Miss the builtin is read-only-stable and
// must NOT be decorated. The positive case (Intersection) is pinned by
// `builtin-volatile-raytmax-intersection.slang` /
// `builtin-volatile-raytmax-vk-mem-model.slang`; this test pins the
// negative case from one representative stage (closesthit) under both
// memory models. The same `CHECK-NOT: Volatile` rejects either form
// (variable-level decoration under GLSL450, per-access mask under
// vk_mem_model).

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry closestHitMain -stage closesthit -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry closestHitMain -stage closesthit -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<float> output;

struct Payload { float v; };
struct Attribs { float2 b; };

// CHECK: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn RayTmaxKHR
// CHECK-NOT: Volatile
[shader("closesthit")]
void closestHitMain(inout Payload p, in Attribs a)
{
    p.v = RayTCurrent();
}
