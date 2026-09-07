//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#implicit-lookup
//META: doc_section_digest=81444a04e3158afb0f979b5eaa4e66672a28128589f884b354ac44b03dbff7ad
//META: purpose=A bare identifier resolves to a field declared inside a global-scope tbuffer, via implicit tbuffer lookup.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a bare identifier inside a compute shader resolves to a field of
// a global-scope `tbuffer` via implicit tbuffer lookup. The bare name `multiplier`
// must resolve to `TParams.multiplier` without qualification. Each back-end lowers
// a tbuffer differently (HLSL keeps `tbuffer`, Metal uses `texture_buffer<>`,
// GLSL/WGSL fall back to a uniform block), so the claim is checked on every shader
// text target the back-end can express it on. `-target spirv-asm` is recorded as an
// Untested `unsupported-on-target` row in the bundle README: the SPIR-V emitter has
// no lowering for the `TextureBuffer` global and aborts with an internal error.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

tbuffer TParams
{
    int multiplier;
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    // 'multiplier' is a bare identifier resolving via implicit tbuffer lookup.
    outBuf[0] = multiplier * 2;
}

// HLSL keeps the `tbuffer` construct and qualifies the bare name through the
// parameter-group instance.
//HLSL-DAG: tbuffer TParams_{{[0-9]+}} :
//HLSL-DAG: TParams_{{[0-9]+}}.multiplier_{{[0-9]+}}

// GLSL lowers the tbuffer to a std140 uniform block; the bare name still reads
// through that block instance.
//GLSL-DAG: {{^}}layout(std140) uniform block_
//GLSL-DAG: TParams_{{[0-9]+}}.multiplier_{{[0-9]+}}

// Metal lowers the tbuffer to a `texture_buffer<>` bound at a texture slot; the
// bare name emits as a field read through that pointer.
//METAL-DAG: texture_buffer<
//METAL-DAG: TParams_{{[0-9]+}}->multiplier_{{[0-9]+}}

// WGSL binds the parameter group as a `var<uniform>` struct and qualifies the read.
//WGSL-DAG: var<uniform> TParams_{{[0-9]+}} :
//WGSL-DAG: TParams_{{[0-9]+}}.multiplier_{{[0-9]+}}
