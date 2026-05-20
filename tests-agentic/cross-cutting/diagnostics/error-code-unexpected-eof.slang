//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: an unterminated struct body produces an E20001 unexpected-token error pointing at end-of-file.
//META: intent=negative
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
