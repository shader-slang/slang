//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=b22ae0db87266fb01a1be1ff4487019df27480bf09ff6476d416556c6388f8d4
//META: purpose=Member lookup on an ErrorType silently returns empty so a downstream member access does not cascade into a 'member not found' diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's failure-modes section: "Member lookup on `ErrorType`.
// `_lookUpMembersInType` reaches the type-shape dispatch and falls
// through without adding any items — lookup silently returns empty
// so that a downstream error does not cascade into 'unknown member'
// noise." Verify by accessing `.field` on an undefined name: only
// the `undefined identifier` diagnostic fires, *not* a follow-on
// `'field' is not a member of 'ErrorType'`. We use exhaustive
// DIAGNOSTIC_TEST mode to catch any unexpected cascading diag.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int n = noSuchVar.field;
//CHECK:    ^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^ undefined identifier 'noSuchVar'.
// CHECK: E30015
}
