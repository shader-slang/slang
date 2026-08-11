//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=53cc9594629df0d6fdaba40b6892c4ce92119b3c38da01e7da3565a3cdc7634f
//META: purpose=A function parameter lives in its owning CallableDecl's scope; a sibling function cannot see it.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `square`'s parameter `n` lives in square's scope only. `main`'s
// scope chain does not pass through square's scope, so `n` is not
// resolvable from main.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int square(int n) { return n * n; }

void main()
{
    int r = square(7);
    int s = n;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'n'.
// CHECK: E30015
}
