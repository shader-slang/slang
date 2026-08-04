//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#option-set-toggles
//META: doc_section_digest=d95eda883eb0804ab1b80bcfd1567b5546c822286eb81da20bfc48d4944ed849
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

//TEST:SIMPLE(filecheck=GLSL):-target hlsl -line-directive-mode glsl -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
}

//GLSL: #line {{[0-9]+}} 0
