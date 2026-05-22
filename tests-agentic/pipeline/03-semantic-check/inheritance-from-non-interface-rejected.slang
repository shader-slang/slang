//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Slang's inheritance checker rejects a struct attempting to inherit from a built-in non-interface type, naming the offending base type.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Files-and-responsibilities table lists slang-check-inheritance.cpp
// as the file that handles inheritance. Verify the user-visible
// consequence when a struct tries to inherit from a sealed builtin
// type: the checker emits a diagnostic citing the base type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct B : int {}
//CHECK:   ^^^ cannot inherit from type 'int' in module 'core' because it is implicitly 'sealed'
// CHECK: E30830
// CHECK: E30831

void main() {}
