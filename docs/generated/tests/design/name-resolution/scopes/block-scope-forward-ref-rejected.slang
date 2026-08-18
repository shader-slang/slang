//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=b57e81781278bd6752bd817f50c238aae4f56d4fda646e22befd826b34d6cab4
//META: purpose=Inside a block the `Decl::hiddenFromLookup` flag hides a local until its DeclStmt; a forward reference inside the same block is rejected as undefined.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "Empty block scope" / hiddenFromLookup explanation: the flag is set
// on entry to a block and cleared one decl at a time. A use that
// precedes the decl sees the local as hidden -> undefined.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int y = x + 1;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'x'.
// CHECK: E30015
    int x = 5;
}
