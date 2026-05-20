//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr
//META: doc_section_digest=4254abda0523877f7fb67acc9da1ad7de3884b6d869e71f92f12575587230f39
//META: purpose=A TupleTypeExpr `(T1, T2)` is a tuple-type spelling; declaring a tuple-typed variable and accessing the i-th element via _i works.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists TupleTypeExpr as a type-expression-family node for
// `(T1, T2, ...)` tuple-type spellings. A tuple is constructed with
// makeTuple and read via the ._i indexed accessors.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Tuple<int, float> t = makeTuple(7, 1.5f);
    // CHECK: a=7
    printf("a=%d\n", t._0);
    // CHECK: b=1
    printf("b=%d\n", int(t._1));
}
