//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: an unterminated `#if` produces E15000 plus a "see 'if' directive" note pointing back at the opener.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `end-of-file-in-preprocessor-conditional` (15000) to its
// rendered code. The diag chain also emits a note pointing back
// at the originating `#if` — verify both via FileCheck.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl

#if 1
void main() {}

// CHECK: E15000
// CHECK: end of file encountered during preprocessor conditional
// CHECK: see 'if' directive
