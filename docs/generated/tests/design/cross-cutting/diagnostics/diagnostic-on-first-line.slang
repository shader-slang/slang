//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=b56b816820af1c15450b34e955f5435969bcf8f2936c6dbb068f8ab21ec3dca3
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

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -entry main -stage compute

#line 1
int g = nonexistentGlobal;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {}

// CHECK: :1:
// CHECK: undefined identifier 'nonexistentGlobal'
