//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#option-set-toggles
//META: doc_section_digest=d95eda883eb0804ab1b80bcfd1567b5546c822286eb81da20bfc48d4944ed849
//META: purpose=Verifies -line-directive-mode none removes all `#line` directives from HLSL output while the default emits C-style `#line N "file"`.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of OptionKind::LineDirectiveMode (-line-directive-mode).
// With `none`, the emitter omits every `#line` directive; the default
// (unspecified) emits C-style `#line <n> "<file>"`. The DEFAULT directive
// pins the presence of a C-style `#line` line (filename wildcarded since it is
// the on-disk source name); the NONE directive uses CHECK-NOT to pin absence.

// Emission fan-out: the suppression half of the claim is target-independent —
// the shared C-like emitter drops every `#line` under `none` — so GLSL and
// Metal each get a default/none pair. Their defaults differ from HLSL's:
// GLSL's default is already the GLSL numeric-id form (`#line N 0`), so its
// default directive pins that form instead of the C-style one. WGSL emits no
// `#line` at all by default, so `none` produces no observable delta there, and
// spirv-asm never emits `#line`; both are recorded in the README
// Untested-claims table rather than given a vacuously-passing CHECK-NOT.

//TEST:SIMPLE(filecheck=DEFAULT):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=NONE):-target hlsl -line-directive-mode none -entry main -stage compute
//TEST:SIMPLE(filecheck=GLDEF):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSUP):-target glsl -line-directive-mode none -entry main -stage compute
//TEST:SIMPLE(filecheck=MTDEF):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=MTSUP):-target metal -line-directive-mode none -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
}

//DEFAULT: #line {{[0-9]+}} "{{.*}}"
//NONE-NOT: #line

//GLDEF: #line {{[0-9]+}} 0
//GLSUP-NOT: #line

//MTDEF: #line {{[0-9]+}} "{{.*}}"
//MTSUP-NOT: #line
