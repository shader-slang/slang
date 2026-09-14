//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=f952036c11ff9b8c1d73b32cb9eb2592525bdd2e44c5b243e4cfdec0a9352108
//META: purpose=Boundary: a diagnostic on line 1 (the lower edge of the line-number axis) still renders `:1:` in the decoded triple.
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
