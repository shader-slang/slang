//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#stage-specific-entry-point-attributes
//META: doc_section_digest=9976dfbcc03a512351f63ab9251ef58411b90efdfa2f7539d1c820164c50911b
//META: purpose=The `[shader("compute")]` attribute marks a function as a compute entry point selectable without `-entry`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the stage string in the attribute is what makes the function
// discoverable as an entry point: the compile names neither `-entry` nor
// `-stage`, so nothing else identifies `csMain`. The CHECK-DAG lines assert the
// function and its compute workgroup attribute reach the emitted HLSL.

// The same entry-point discovery is repeated on every other shader back end,
// each of which then stamps its own compute-entry marker: GLSL a
// `local_size_*` input layout, Metal `[[kernel]]`, WGSL `@compute`, and SPIR-V
// a `GLCompute` `OpEntryPoint`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl
//TEST:SIMPLE(filecheck=GLSL):-target glsl
//TEST:SIMPLE(filecheck=METAL):-target metal
//TEST:SIMPLE(filecheck=WGSL):-target wgsl
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void csMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = 7;
}

//CHECK-DAG: numthreads
//CHECK-DAG: csMain

// GLSL renames every entry point to `main`, so the compute marker is what shows
// the attribute selected the function.
//GLSL: layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//METAL: {{\[\[}}kernel{{\]\]}} void csMain
//WGSL-DAG: @compute
//WGSL-DAG: fn csMain
//SPIRV: OpEntryPoint GLCompute %csMain
