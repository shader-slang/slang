//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=984d93f961836cc85daa4d28cbdc53dade2964991d8474fd53b87f15b712e265
//META: purpose=ExtensionDecl members are reachable only via member lookup on the extended type, never as unqualified file-scope names.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "ExtensionDecl members are not in the extension's scope chain"
// edge case: from outside Point's member-lookup, `manhattan` is not
// visible by unqualified name even though an extension provides it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Point { int x; int y; }

extension Point {
    int manhattan() { return x + y; }
}

void main()
{
    Point p = {3, 5};
    int m = manhattan();
//CHECK:    ^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^ undefined identifier 'manhattan'.
}
