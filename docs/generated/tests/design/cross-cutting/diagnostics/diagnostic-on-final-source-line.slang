//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=f952036c11ff9b8c1d73b32cb9eb2592525bdd2e44c5b243e4cfdec0a9352108
//META: purpose=Boundary: a diagnostic on the file's final source line still resolves to a real source line and renders a caret.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Edge of source-location decoding at the upper file boundary:
// the offending token (an unterminated struct body) lives on the
// last line of the file. The renderer must still locate the
// source file and emit a caret-aligned diagnostic.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl
void main() {}
struct EndingStruct {
// CHECK: E20001
// CHECK: diagnostic-on-final-source-line.slang
