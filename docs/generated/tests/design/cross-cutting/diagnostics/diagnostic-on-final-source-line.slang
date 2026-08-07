//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=c0e97396ea139ceab54212053b6d443692d19f0faf2847c4dc505373ae8c4861
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
