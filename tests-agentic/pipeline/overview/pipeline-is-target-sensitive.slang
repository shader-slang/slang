//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/overview.md#ir-passes
//META: doc_section_digest=8092386cefcada1f1af009b686bd9a12889a2fda1b578a422ef69221f6ff7c43
//META: purpose=The IR pass list is target-sensitive; the same source emits target-shaped output (not just textually identical) for each backend.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the overview claim that "the IR pass list is
// target-sensitive". A single source shader, with the same
// numthreads attribute, emerges as syntactically different text in
// HLSL ([numthreads(1, 1, 1)]), GLSL (layout(local_size_x = 1, ...)),
// and SPIR-V (OpExecutionMode %main LocalSize 1 1 1). If the IR
// passes did not branch on target, the emitter would not be able to
// produce these target-shaped forms.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // HLSL: numthreads(1, 1, 1)
    // GLSL: local_size_x = 1, local_size_y = 1, local_size_z = 1
    // SPIRV: OpExecutionMode %main LocalSize 1 1 1
    return;
}
