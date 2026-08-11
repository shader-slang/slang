//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=c07669430fa1f58c4db53ae38c50d1523f8399ab4e7aa698054222ac63e9d41b
//META: purpose=An unterminated `#if` renders E15000 plus a note pointing back at the opening directive.
//META: intent=functional
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
