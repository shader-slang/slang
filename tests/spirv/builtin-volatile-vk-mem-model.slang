// builtin-volatile-vk-mem-model.slang
//
// Regression test for shader-slang/slang#10528.
// Under the Vulkan memory model the variable-level `OpDecorate Volatile`
// is invalid; the corresponding `Volatile` semantics for subgroup
// builtins in raytracing stages must instead be expressed as the
// `Volatile` memory access mask on each `OpLoad`.
//
// Three raygeneration entry points exercise two independent failure
// modes for the per-access mask injection:
//   - `rayGenMainA` and `rayGenMainB` each read `WaveGetLaneIndex()`
//     once. Both end up in the same SPIR-V module and (with the
//     current IR pipeline) share a single `OpVariable` for
//     `SubgroupLocalInvocationId`. Each entry point emits its own
//     `OpLoad` that must carry the `Volatile` mask — the pre-cache
//     `m_volatileBuiltinGlobalVars.add()` in `getBuiltinGlobalVar`
//     ensures both call sites' IR-side operands are flagged so the
//     mask injection in `emitSPIRVAsm` fires across entry-point
//     boundaries, not just the first one.
//   - `rayGenMainMulti` reads `WaveGetLaneIndex()` twice in a single
//     body. This pins the per-load (not per-builtin) nature of the
//     `case SpvOpLoad:` branch in `emitSPIRVAsm`: a regression that
//     cached the volatile-set lookup at the first load and skipped
//     subsequent loads would silently drop the mask on every load
//     after the first.
// Combined: 4 `OpLoad` instructions, each must carry `Volatile`.
// The exact-4 count assertion plus the trailing not-clause bounds
// the upper end so a regression that doubles the mask emission is
// also caught.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry rayGenMainA -entry rayGenMainB -entry rayGenMainMulti -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<uint> output;

// SPIR-V layout: Annotations come before Types/Variables. Match the
// `OpDecorate ... BuiltIn` line first, capture the variable name, and
// pin the absence of any `OpDecorate %V Volatile` between annotations
// (variable-level `Volatile` is invalid under the Vulkan memory model).
// CHECK: OpDecorate %[[V:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK-NOT: OpDecorate %[[V]] Volatile

// Then the variable declaration in the Types/Variables section.
// CHECK: %[[V]] = OpVariable {{.*}} Input

// All four `OpLoad`s of %V (one each from rayGenMainA / rayGenMainB,
// plus two from rayGenMainMulti) must carry the Volatile memory
// access mask. The exact count plus the trailing not-clause pins
// both bounds: a regression that drops the mask on later loads
// (cross-entry or intra-entry repeats) fails the count, and a
// regression that doubles the emission fails the trailing not.
// CHECK-COUNT-4: OpLoad {{.*}} %[[V]] {{.*}}Volatile
// CHECK-NOT: OpLoad {{.*}} %[[V]] {{.*}}Volatile

[shader("raygeneration")]
void rayGenMainA()
{
    output[0] = WaveGetLaneIndex();
}

[shader("raygeneration")]
void rayGenMainB()
{
    output[1] = WaveGetLaneIndex();
}

[shader("raygeneration")]
void rayGenMainMulti()
{
    output[2] = WaveGetLaneIndex();
    output[3] = WaveGetLaneIndex();
}
