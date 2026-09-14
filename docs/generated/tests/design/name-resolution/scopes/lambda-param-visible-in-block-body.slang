//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
//META: purpose=A lambda parameter is visible throughout the block form of the body, not just the single-expression form, because the pushed parameter scope is popped only after the body.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the block-body branch of the lambda parameter-scope rule. The
// parameter scope is pushed before the parameter list and popped after the
// body, so it must cover a braced body — including a body that introduces its
// own nested block scope for `t`. The lambda computes `(p + 1) * c`, which
// needs `p` from the parameter scope and `c` from the enclosing function
// through that scope's parent link; `r=18` for p=5, c=3 is reachable only if
// both bind. The single-expression form is covered by
// lambda-param-visible-in-lambda-body.slang.

//TEST:INTERPRET(filecheck=CHECK):

int applyF<F : IFunc<int, int>>(F f, int v)
{
    return f(v);
}

void main()
{
    int c = 3;
    int r = applyF((int p) => { int t = p + 1; return t * c; }, 5);
    // CHECK: r=18
    printf("r=%d\n", r);
}
