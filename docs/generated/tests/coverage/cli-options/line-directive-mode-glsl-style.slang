//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#option-set-toggles
//META: doc_section_digest=65ae1152b296463b8293b7591a32f87fe87ce08c5528a74ea4488483c5c3c5b7
//META: purpose=Verifies -line-directive-mode glsl emits GLSL-style numeric-file-id `#line N 0` directives rather than C-style `#line N "file"`.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of -line-directive-mode glsl. GLSL `#line` directives carry
// a numeric file id instead of a quoted filename, so the same source emits
// `#line 3 0` under `glsl` mode where C-style mode emits `#line 3 "<file>"`.
// CHECK pins the GLSL numeric-id form; this exercises the line-directive-mode
// value-parse and emitter branch driven entirely by the CLI option.

// Emission fan-out: `-line-directive-mode` is read by the shared C-like source
// emitter, so every textual target honours it. GLSL already defaults to the
// numeric-id form, so its directive pins the form rather than a delta; Metal
// defaults to the C-style quoted-filename form, so it gets a paired default run
// (MTCSTY) that shows the delta; WGSL emits no `#line` at all by default, so
// the numeric-id directives appearing under `glsl` mode is itself the delta.
// spirv-asm never emits `#line` (debug info is carried by OpLine under `-g`),
// so it is recorded in the README Untested-claims table.

//TEST:SIMPLE(filecheck=GLSL):-target hlsl -line-directive-mode glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLNUM):-target glsl -line-directive-mode glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=MTNUM):-target metal -line-directive-mode glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=MTCSTY):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGNUM):-target wgsl -line-directive-mode glsl -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
}

//GLSL: #line {{[0-9]+}} 0

//GLNUM: #line {{[0-9]+}} 0
//MTNUM: #line {{[0-9]+}} 0
//MTCSTY: #line {{[0-9]+}} "{{.*}}"
//WGNUM: #line {{[0-9]+}} 0
