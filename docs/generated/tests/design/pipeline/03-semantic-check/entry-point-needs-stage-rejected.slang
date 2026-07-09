//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=ab729adc5c4bfaabc81344dfe31170841f8c87ee52d7df58f2988f73f8d33f7f
//META: purpose=Entry-point checks require a stage attribute on the function or a -stage command-line option; without either the checker emits diagnostic 38007.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Shader-specific checks section says
// slang-check-shader.cpp validates "the function's stage attribute".
// Without one, compilation of the entry point is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry myEntry

void myEntry() {}
/*CHECK:
     ^^^^^^^ no stage specified for entry point
*/
// CHECK: E38007
