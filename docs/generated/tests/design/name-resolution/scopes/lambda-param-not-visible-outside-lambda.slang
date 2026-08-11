//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=53cc9594629df0d6fdaba40b6892c4ce92119b3c38da01e7da3565a3cdc7634f
//META: purpose=The scope holding a lambda's parameter list is owned by the lambda expression, so the parameter name is undefined in the enclosing function after the lambda.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the boundary of the lambda parameter scope. The lambda binds `p`
// and is consumed normally on the preceding line; the next statement in the
// enclosing function reads `p` and must not resolve, because the scope that
// held it was popped with the lambda expression. The annotated E30015 at the
// bare `p` is the observation.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int applyF<F : IFunc<int, int>>(F f, int v)
{
    return f(v);
}

void main()
{
    int r = applyF((int p) => p * 2, 5);
    int q = p;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'p'.
// CHECK: E30015
    printf("%d\n", r + q);
}
