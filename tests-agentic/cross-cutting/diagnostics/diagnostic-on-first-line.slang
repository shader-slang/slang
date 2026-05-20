//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=d4c1621bec12a7398cb1a9674411d74b7313361b2b6d60447dbaae79eb52f58e
//META: purpose=Boundary: a diagnostic whose offending token sits on a `#line 1`-remapped line (numeric edge of the line-number axis) still renders with `:1:` in the file:line:column triple.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Edge of the source-location decoding for the line-number axis:
// use `#line 1` to remap the next physical line back to logical
// line 1, then trigger an undefined-identifier on that line. The
// renderer must emit the remapped `:1:` in the rendered location,
// confirming the line-number boundary is correctly carried through
// SourceManager.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

#line 1
int g = nonexistentGlobal;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {}

// CHECK: :1:
// CHECK: undefined identifier 'nonexistentGlobal'
