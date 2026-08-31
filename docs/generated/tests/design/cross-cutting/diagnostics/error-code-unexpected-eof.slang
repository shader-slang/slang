//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=e5426f9afa17becb9e7d33c23301014e082fad12bbf8aff1c4a169d90575556e
//META: purpose=A lexer/parser-range code (E20001) is rendered for an unterminated struct body.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin the parse-stage diagnostic id for an unterminated struct.
// The compiler emits E20001 "unexpected end of file, expected '}'".
// We use a SIMPLE FileCheck so we can match the code on a
// non-source line (EOF position).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl
struct S {

// CHECK: E20001
// CHECK: unexpected end of file, expected '}'
