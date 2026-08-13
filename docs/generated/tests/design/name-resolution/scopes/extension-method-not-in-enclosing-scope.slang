//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=b57e81781278bd6752bd817f50c238aae4f56d4fda646e22befd826b34d6cab4
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
// CHECK: E30015
}
