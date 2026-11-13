//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=A lexer/parser-range code (E20001) is rendered for an unterminated struct body.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin the parse-stage diagnostic id for an unterminated struct.
// The compiler emits E20001 "unexpected end of file, expected '}'".
// We use a SIMPLE FileCheck so we can match the code on a
// non-source line (EOF position).

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl
struct S {

// CHECK: E20001
// CHECK: unexpected end of file, expected '}'
