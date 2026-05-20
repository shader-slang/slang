//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=A LambdaDecl is the closure-struct produced by a lambda expression; the closure is callable through the IFunc-constrained generic helper.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of LambdaDecl. The `=>` expression
// produces a closure value; an IFunc-constrained helper invokes the
// closure and returns its result.

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
