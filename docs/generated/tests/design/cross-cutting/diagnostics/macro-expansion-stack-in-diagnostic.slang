//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=b56b816820af1c15450b34e955f5435969bcf8f2936c6dbb068f8ab21ec3dca3
//META: purpose=A diagnostic raised inside a macro expansion still decodes to a source location and renders the expanded identifier in its message.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a diagnostic raised inside a macro expansion still decodes to
// a usable source location and carries the expanded spelling. A macro body
// naming an undeclared identifier is expanded in the entry point; CHECK
// asserts the undefined-identifier message and the expanded name, which the
// sink can only produce by decoding the expansion's SourceLoc.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -entry main -stage compute

#define USE_UNDECLARED nonexistentVar

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = USE_UNDECLARED;
}

// CHECK: undefined identifier
// CHECK: nonexistentVar
