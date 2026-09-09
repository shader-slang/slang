//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=e5426f9afa17becb9e7d33c23301014e082fad12bbf8aff1c4a169d90575556e
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
