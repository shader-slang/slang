// builtin-no-volatile-raytmax.slang
//
// Regression test for shader-slang/slang#10528. `RayTCurrent()` lowers
// to `OpLoad builtin(RayTmaxKHR)`. The Vulkan SPIR-V env spec mandates
// `Volatile` semantics on `RayTmaxKHR` only in the Intersection stage;
// in AnyHit, ClosestHit, and Miss the builtin is read-only-stable and
// must NOT be decorated. The positive case (Intersection) is pinned by
// `builtin-volatile-raytmax-intersection.slang` /
// `builtin-volatile-raytmax-vk-mem-model.slang`; this test pins the
// negative case from each of the three non-Intersection RT stages
// (anyhit, closesthit, miss) under both memory models. Three stages ×
// two memory models = six directives. A regression that widened the
// gate (e.g. swapped `Stage::Intersection` for `isRaytracingStage(...)`)
// would silently regress on the AnyHit / Miss paths if those stages
// weren't explicitly exercised — closesthit alone is insufficient.
// The same `CHECK_*-NOT: Volatile` rejects either failure form per
// stage (variable-level decoration under GLSL450, per-access mask
// under vk_mem_model).

//TEST:SIMPLE(filecheck=CHECK_CH): -target spirv-asm -entry closestHitMain -stage closesthit -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_CH): -target spirv-asm -entry closestHitMain -stage closesthit -emit-spirv-directly -capability vk_mem_model
//TEST:SIMPLE(filecheck=CHECK_AH): -target spirv-asm -entry anyHitMain -stage anyhit -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_AH): -target spirv-asm -entry anyHitMain -stage anyhit -emit-spirv-directly -capability vk_mem_model
//TEST:SIMPLE(filecheck=CHECK_MISS): -target spirv-asm -entry missMain -stage miss -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_MISS): -target spirv-asm -entry missMain -stage miss -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<float> output;

struct Payload { float v; };
struct Attribs { float2 b; };

// Each stage's `CHECK_*-NOT` brackets the positive `CHECK_*`: leading
// `-NOT` scans file start → match, trailing scans match → EOF, so the
// `Volatile` absence is asserted across the whole disassembly per
// stage.
// CHECK_CH-NOT: Volatile
// CHECK_CH: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn RayTmaxKHR
// CHECK_CH-NOT: Volatile
[shader("closesthit")]
void closestHitMain(inout Payload p, in Attribs a)
{
    p.v = RayTCurrent();
}

// CHECK_AH-NOT: Volatile
// CHECK_AH: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn RayTmaxKHR
// CHECK_AH-NOT: Volatile
[shader("anyhit")]
void anyHitMain(inout Payload p, in Attribs a)
{
    p.v = RayTCurrent();
}

// CHECK_MISS-NOT: Volatile
// CHECK_MISS: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn RayTmaxKHR
// CHECK_MISS-NOT: Volatile
[shader("miss")]
void missMain(inout Payload p)
{
    p.v = RayTCurrent();
}
