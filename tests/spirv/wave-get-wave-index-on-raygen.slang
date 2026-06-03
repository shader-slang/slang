// Regression test for #11303: `WaveGetWaveIndex()` and `WaveGetNumWaves()`
// previously emitted spec-invalid SPIR-V when used from a raytracing
// entry point. `BuiltIn SubgroupId` and `BuiltIn NumSubgroups` are
// restricted to compute-class execution models per
// VUID-SubgroupId-SubgroupId-04367 / VUID-NumSubgroups-NumSubgroups-04293,
// so generated SPIR-V referencing them from `RayGenerationKHR`,
// `ClosestHitKHR`, etc. is rejected by spirv-val.
//
// `source/slang/hlsl.meta.slang` now wraps each `case spirv:` arm in a
// `__stage_switch` accepting `compute` / `_mesh` / `_amplification`,
// emitting a `static_assert` for any other stage. The
// `tests/spirv/wave-*.slang` family exercises the compute path; this
// test pins the negative path on a raytracing entry point.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry main -stage raygeneration -emit-spirv-directly

RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
{
    output[0] = WaveGetWaveIndex();
//CHECK: WaveGetWaveIndex is only supported in compute-class execution models for SPIR-V
    output[1] = WaveGetNumWaves();
//CHECK: WaveGetNumWaves is only supported in compute-class execution models for SPIR-V
}
