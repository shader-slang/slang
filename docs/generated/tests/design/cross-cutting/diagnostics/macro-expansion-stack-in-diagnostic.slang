//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=c0e97396ea139ceab54212053b6d443692d19f0faf2847c4dc505373ae8c4861
//META: purpose=A diagnostic raised inside a macro expansion still decodes to a source location and renders the expanded identifier in its message.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a diagnostic raised inside a macro expansion still decodes to
// a usable source location and carries the expanded spelling. A macro body
// naming an undeclared identifier is expanded in the entry point; CHECK
// asserts the undefined-identifier message and the expanded name, which the
// sink can only produce by decoding the expansion's SourceLoc.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

#define USE_UNDECLARED nonexistentVar

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = USE_UNDECLARED;
}

// CHECK: undefined identifier
// CHECK: nonexistentVar
