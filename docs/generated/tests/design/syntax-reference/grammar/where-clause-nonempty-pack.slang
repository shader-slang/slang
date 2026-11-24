//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=41260c8f8bf7f0e8598932b33e0e91618368529d9e8fe3e7b7881fe0dd2a078f
//META: purpose=`WhereTerm` accepts the non-empty pack constraint `'nonempty' '(' Expr ')'` on a variadic `each` parameter.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `nonempty(Pack)` where-term: a generic with a variadic
// `each T` parameter constrained non-empty specializes fine with an
// explicit two-type argument list, so the constraint parsed and is
// satisfied by a non-empty pack.

//TEST:INTERPRET(filecheck=CHECK):

int packOk<each T>() where nonempty(T)
{
    return 11;
}

void main()
{
    // CHECK: r=11
    printf("r=%d\n", packOk<int, float>());
}
