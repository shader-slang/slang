//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=d4c1621bec12a7398cb1a9674411d74b7313361b2b6d60447dbaae79eb52f58e
//META: purpose=Boundary: a diagnostic whose offending token sits on the final non-newline line of the file still renders with a correctly-aligned caret and named file.
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
