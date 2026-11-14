//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=7b8fdedc5a4069b7a10da3f6dcaf8b3268c6f74fbb85dd4ea2edff960fab2b8b
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
