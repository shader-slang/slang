// builtin-volatile-mixed-stages.slang
//
// Regression test for shader-slang/slang#10528.
// A single Slang module with both `cs()` (compute) and `rg()`
// (raygeneration) entry points reading `WaveGetLaneIndex()`. Pins the
// observed end-to-end SPIR-V emit behaviour so that a regression in
// either the IR-side referencing-entry-points propagation or the
// `BuiltinSpvVarKey` keying would be visible:
//
// With the current IR pipeline, the `SPIRVAsmOperandBuiltinVar` is
// shared / referenced from both entry points. Because at least one
// referencing stage (`raygeneration`) is in the raytracing-volatile
// set, `needVolatileDecorationForBuiltinVar` returns true and the
// single emitted `OpVariable` is decorated `Volatile` under the
// GLSL450 memory model. The `OpEntryPoint` for the compute entry
// point therefore inherits `Volatile` semantics on its load —
// overconservative but spec-correct. `BuiltinSpvVarKey` is keyed on
// (builtin, storage class, flat, pointee type); if a future IR pass
// kept distinct builtin operands per call-site, this test would need
// to assert two `OpVariable`s instead of one.
//
// What this test pins, regardless of which side of that split the
// pipeline lands on:
// - the `BuiltIn SubgroupLocalInvocationId` decoration is present;
// - the `Volatile` decoration is present (not lost by an over-eager
//   compute-only filter);
// - both entry points compile cleanly and reference a builtin in
//   their `OpEntryPoint` interface list.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry cs -entry rg -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_VK): -target spirv-asm -entry cs -entry rg -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<uint> output;

// === GLSL450 path ===
// Both entry points are emitted and each lists `SubgroupLocalInvocationId`
// in its `OpEntryPoint` interface.
// CHECK: OpEntryPoint GLCompute %{{[A-Za-z0-9_]+}} "cs" {{.*}}SubgroupLocalInvocationId
// CHECK: OpEntryPoint RayGenerationKHR %{{[A-Za-z0-9_]+}} "rg" {{.*}}SubgroupLocalInvocationId

// Exactly one `OpVariable` for `SubgroupLocalInvocationId` is emitted,
// and that one variable is decorated `Volatile`. The raygen path
// requires the `Volatile` decoration; if the cache axis or stage-set
// logic were to drop it, this would fail.
//
// The PR drops `isVolatile` from the cache key on the assumption that
// all references to a given builtin share one IR-side
// `IRSPIRVAsmOperandBuiltinVar`; if a future IR change splits per
// call-site operands, the cache would emit two variables and the
// volatile decoration could land on only the second one (silent
// regression on the compute-side load). The exact-1 count + the
// trailing not-clause pins this single-variable invariant. The
// captured `[[V]]` is then reused for the `Volatile` decoration so
// all assertions tie back to the same one variable.
// CHECK-COUNT-1: OpDecorate %[[V:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK-NOT: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK: OpDecorate %[[V]] Volatile

// === vk_mem_model path ===
// Same shared `OpVariable` for both stages, but the `Volatile` semantic
// surfaces as the per-`OpLoad` memory-access mask rather than a variable
// decoration. Since the IR-side flagging is keyed on the union of
// referencing entry points (raygen pulls the operand into the volatile
// set; compute side then shares it), BOTH `OpLoad`s carry the mask —
// overconservative on the compute side but spec-correct, mirroring the
// GLSL450 inheritance. The exact-2 count plus the trailing not-clause
// pins the upper bound so a regression that flips the disambiguation in
// either direction (no loads, or three loads) is caught.
// CHECK_VK: OpEntryPoint GLCompute %{{[A-Za-z0-9_]+}} "cs" {{.*}}SubgroupLocalInvocationId
// CHECK_VK: OpEntryPoint RayGenerationKHR %{{[A-Za-z0-9_]+}} "rg" {{.*}}SubgroupLocalInvocationId
// Exactly one BuiltIn SubgroupLocalInvocationId decoration (single-
// OpVariable invariant under vk_mem_model too); no variable-level
// Volatile on it; both OpLoads of it carry the per-access mask.
// CHECK_VK-COUNT-1: OpDecorate %[[V_VK:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK_VK-NOT: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK_VK-NOT: OpDecorate %[[V_VK]] Volatile
// CHECK_VK-COUNT-2: OpLoad {{.*}} %[[V_VK]] Volatile
// CHECK_VK-NOT: OpLoad {{.*}} %[[V_VK]] Volatile

[numthreads(64, 1, 1)]
[shader("compute")]
void cs()
{
    output[0] = WaveGetLaneIndex();
}

[shader("raygeneration")]
void rg()
{
    output[1] = WaveGetLaneIndex();
}
