//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:30:40+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=a79d944b7d2a39d014afe31606a0e3f7d96f6912c534eadd8d9b1160507f3183
//META: purpose=A later directive naming a different source language than the first selection (here `#version` selecting GLSL, then `#lang slang`) is diagnosed as a conflict (E00121).
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#version` records GLSL through the same first-selection rule as `#lang`.
// A subsequent `#lang slang 2026` names a different language, so
// `_tryApplySourceLanguageDirective` diagnoses
// `ConflictingSourceLanguageDirectives` (E00121) and keeps the first selection.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -entry main -stage compute

#version 450
#lang slang 2026

void main() {}

// CHECK: E00121
// CHECK: first source-language directive
