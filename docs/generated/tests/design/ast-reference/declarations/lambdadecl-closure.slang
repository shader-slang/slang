//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A lambda expression produces a callable closure declaration that an IFunc-constrained generic can invoke.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the checker turns a lambda expression into a callable value.
// The lambda is passed to a generic constrained on `IFunc`, which invokes it;
// the CHECK asserts the lambda body's result, so the closure must have been
// materialized as a conforming callable.

//TEST:INTERPRET(filecheck=CHECK):

TResult apply<TResult, TFunc:IFunc<TResult, int>>(TFunc f)
{
    return f(7);
}

void main()
{
    let r = apply( (int x) => x * 3 );
    //CHECK: 21
    printf("%d\n", r);
}
