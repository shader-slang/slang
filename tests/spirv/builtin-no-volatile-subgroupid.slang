// builtin-no-volatile-subgroupid.slang
//
// Regression test for shader-slang/slang#10528 — SubgroupId exclusion.
//
// `needVolatileDecorationForBuiltinVar` deliberately OMITS
// `SpvBuiltInSubgroupId` from the volatile-required switch (anchored
// at `slang-emit-spirv.cpp:6935-6946`). The omission is per Vulkan
// SPIR-V env spec VUID-SubgroupId-SubgroupId-04367, which restricts
// `BuiltIn SubgroupId` to compute-class execution models — adding
// `Volatile` to it under a raytracing stage doesn't make the SPIR-V
// any more valid (VUID-04367 fires regardless of `Volatile`); the
// usability of `WaveGetWaveIndex()` in RT stages is tracked
// separately as #11303.
//
// A future contributor "completing" the volatile switch by adding
// `case SpvBuiltInSubgroupId:` (a plausible mechanical refactor)
// would not fail any of the existing 11 tests, because none of them
// reaches `BuiltIn SubgroupId` from a raytracing-volatile stage.
// This test pins that omission directly via a hand-rolled `spirv_asm`
// block (the same style `builtin-volatile-smidnv-warpidnv.slang`
// uses to reach `SMIDNV`/`WarpIDNV`, which also have no Slang
// intrinsic surface). `SubgroupId` from raygen is invalid SPIR-V
// per VUID-04367, so the directives carry `-skip-spirv-validation`
// — the regression we're guarding against is the appearance of
// `Volatile` decoration on a builtin Slang shouldn't be flagging in
// the first place, not the underlying VUID.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry rayGenMain -stage raygeneration -emit-spirv-directly -skip-spirv-validation
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry rayGenMain -stage raygeneration -emit-spirv-directly -capability vk_mem_model -skip-spirv-validation

RWStructuredBuffer<uint> output;

uint readSubgroupId()
{
    return spirv_asm
    {
        OpCapability GroupNonUniform;
        result:$$uint = OpLoad builtin(SubgroupId:uint)
    };
}

// `BuiltIn SubgroupId` decoration must appear (the variable IS emitted
// — Slang doesn't gatekeep the underlying SPIR-V validity), but neither
// `OpDecorate %V Volatile` nor any `OpLoad ... Volatile` against this
// variable may appear under either memory model.
// CHECK: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupId
// CHECK-NOT: Volatile

[shader("raygeneration")]
void rayGenMain()
{
    output[0] = readSubgroupId();
}
