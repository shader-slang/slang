//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=f6e12c33ddd19d9369ee802b5f8dd1f7ec0e74a603076a1958b10edb008910ca
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
