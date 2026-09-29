//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=7b8fdedc5a4069b7a10da3f6dcaf8b3268c6f74fbb85dd4ea2edff960fab2b8b
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
