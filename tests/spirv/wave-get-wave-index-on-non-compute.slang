// Regression test for #11303: `WaveGetWaveIndex()` and `WaveGetNumWaves()`
// previously emitted spec-invalid SPIR-V when used from a non-compute-class
// entry point. `BuiltIn SubgroupId` and `BuiltIn NumSubgroups` are
// restricted to compute-class execution models per
// VUID-SubgroupId-SubgroupId-04367 / VUID-NumSubgroups-NumSubgroups-04293,
// so generated SPIR-V referencing them from `RayGenerationKHR`,
// `Fragment`, etc. is rejected by spirv-val. The same restriction
// applies on `-target glsl` (`gl_NumSubgroups` / `gl_SubgroupID` are
// declared only in `computeSubgroupDecls` per glslang).
//
// `source/slang/hlsl.meta.slang` now wraps each `case spirv:` and
// `case glsl:` arm in a `__stage_switch` accepting `compute` / `_mesh`
// / `_amplification`, emitting a `static_assert` for any other stage.
// Compute path is exercised by
// `tests/hlsl-intrinsic/wave-get-{wave-index,num-waves}.slang`; the
// mesh/amplification positive paths by
// `tests/spirv/wave-get-wave-index-mesh-stage.slang`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry rayMain -stage raygeneration -emit-spirv-directly
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry pixelMain -stage fragment -emit-spirv-directly
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target glsl -entry rayMain -stage raygeneration

RWStructuredBuffer<uint> output;

void rayMain()
{
    output[0] = WaveGetWaveIndex();
//CHECK: WaveGetWaveIndex is only supported in compute-class execution models
    output[1] = WaveGetNumWaves();
//CHECK: WaveGetNumWaves is only supported in compute-class execution models
}

float4 pixelMain() : SV_Target
{
    return float4(WaveGetWaveIndex(), WaveGetNumWaves(), 0, 0);
}
