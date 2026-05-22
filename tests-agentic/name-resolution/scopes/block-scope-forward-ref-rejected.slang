//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=984d93f961836cc85daa4d28cbdc53dade2964991d8474fd53b87f15b712e265
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
