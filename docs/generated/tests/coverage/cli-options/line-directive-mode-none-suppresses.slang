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

//TEST:SIMPLE(filecheck=DEFAULT):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=NONE):-target hlsl -line-directive-mode none -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
}

//DEFAULT: #line {{[0-9]+}} "{{.*}}"
//NONE-NOT: #line
