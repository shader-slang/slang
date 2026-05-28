// builtin-volatile-combined-intersection.slang
//
// Regression test for shader-slang/slang#10528 — combined subgroup +
// `RayTmaxKHR` in a single intersection entry point.
//
// `needVolatileDecorationForBuiltinVar` has two independent trigger
// paths: the subgroup/SM family (gated on
// `isRaytracingStageRequiringVolatileBuiltins` walking
// `m_referencingEntryPoints`) and the `RayTmaxKHR` branch (gated on
// `isInstUsedInStage(irInst, Stage::Intersection)`). Every other
// regression test exercises one path or the other in isolation; this
// one mixes both in a single body so a regression that interferes
// between them — e.g. an early-return that fires for one family and
// short-circuits the other, or a shared cache slot accidentally
// collapsing two builtins — would be caught.
//
// `intersectionMain` reads three distinct builtins:
//   - `WaveGetLaneIndex()`  → `SubgroupLocalInvocationId`
//   - `WaveGetLaneCount()`  → `SubgroupSize`
//   - `RayTCurrent()`       → `RayTmaxKHR` (Intersection-only)
//
// All three must be flagged volatile in this stage under both memory
// models.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry intersectionMain -stage intersection -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_VK): -target spirv-asm -entry intersectionMain -stage intersection -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<float> output;

// === GLSL450 path ===
// All three variables get `OpDecorate ... Volatile` after the BuiltIn.
// CHECK-DAG: OpDecorate %[[L:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK-DAG: OpDecorate %[[L]] Volatile
// CHECK-DAG: OpDecorate %[[S:[A-Za-z0-9_]+]] BuiltIn SubgroupSize
// CHECK-DAG: OpDecorate %[[S]] Volatile
// CHECK-DAG: OpDecorate %[[T:[A-Za-z0-9_]+]] BuiltIn RayTmaxKHR
// CHECK-DAG: OpDecorate %[[T]] Volatile

// === vk_mem_model path ===
// No variable-level Volatile on any of the three; each `OpLoad` of
// each variable carries the `Volatile` memory-access mask.
// CHECK_VK-DAG: OpDecorate %[[L_VK:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK_VK-DAG: OpDecorate %[[S_VK:[A-Za-z0-9_]+]] BuiltIn SubgroupSize
// CHECK_VK-DAG: OpDecorate %[[T_VK:[A-Za-z0-9_]+]] BuiltIn RayTmaxKHR
// CHECK_VK-NOT: OpDecorate %[[L_VK]] Volatile
// CHECK_VK-NOT: OpDecorate %[[S_VK]] Volatile
// CHECK_VK-NOT: OpDecorate %[[T_VK]] Volatile
// CHECK_VK-DAG: OpLoad {{.*}} %[[L_VK]] {{.*}}Volatile
// CHECK_VK-DAG: OpLoad {{.*}} %[[S_VK]] {{.*}}Volatile
// CHECK_VK-DAG: OpLoad {{.*}} %[[T_VK]] {{.*}}Volatile

[shader("intersection")]
void intersectionMain()
{
    output[0] = float(WaveGetLaneIndex())
              + float(WaveGetLaneCount())
              + RayTCurrent();
}
