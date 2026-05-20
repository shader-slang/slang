//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=6ed31708085f4823fd353a550243a17316068330fb97dc1ed367bc25d6471443
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
