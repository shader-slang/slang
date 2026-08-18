//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=61eae4cb09aaf55ca13e1b5adcdbe1bcd8d876e194e57de10e1861ffeb9396c7
//META: purpose=`SV_WaveIndex` is one of the wave builtins `hlsl.meta.slang` layers in; it is accepted as a compute entry-point parameter semantic and survives into the emitted HLSL signature.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `SV_WaveIndex` semantic name is known to the compiler
// and is not silently dropped. The compute entry takes it alongside
// `SV_GroupIndex` and stores the wave index through the group index so
// neither parameter can be eliminated. CHECK asserts both semantics
// appear on the emitted HLSL entry signature; an unrecognized semantic
// would be rejected during checking instead.
//
// Metal and WGSL map the same semantic onto their own wave-index
// entry-point builtin (`simdgroup_index_in_threadgroup` and
// `@builtin(subgroup_id)` respectively), so the semantic is observable
// there too. GLSL and SPIR-V have no varying-parameter mapping for it
// at all -- see the `unsupported-on-target` rows in the bundle README
// and the matching `undocumented-behavior` doc-gap row.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint wi: SV_WaveIndex, uint gi: SV_GroupIndex)
{
    outBuf[gi] = wi;
}

//HLSL: void main(
//HLSL-SAME: {{SV_WaveIndex|SV_WAVEINDEX}}
//HLSL-SAME: {{SV_GroupIndex|SV_GROUPINDEX}}

//METAL: uint wi{{.*}}{{\[\[}}simdgroup_index_in_threadgroup

//WGSL: @builtin(subgroup_id) wi
//WGSL-SAME: u32
