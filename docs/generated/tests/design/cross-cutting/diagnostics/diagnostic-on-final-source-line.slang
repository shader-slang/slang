//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=b56b816820af1c15450b34e955f5435969bcf8f2936c6dbb068f8ab21ec3dca3
//META: purpose=Boundary: a diagnostic on the file's final source line still resolves to a real source line and renders a caret.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Edge of source-location decoding at the upper file boundary:
// the offending token (an unterminated struct body) lives on the
// last line of the file. The renderer must still locate the
// source file and emit a caret-aligned diagnostic.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl
void main() {}
struct EndingStruct {
// CHECK: E20001
// CHECK: diagnostic-on-final-source-line.slang
