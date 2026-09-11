//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=9fa528801fe0f265a456b36eb8381148d65db73060d29e10a33ff9ce384104ac
//META: purpose=`WhereTerm` accepts the oriented pack-count constraint `'countof' '(' Expr ')' '==' Expr`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `countof(Pack) == N` where-term in its documented
// orientation (countof on the left). A generic constrained to a
// two-type pack specializes with exactly two explicit arguments,
// proving the constraint parsed and was checked.

//TEST:INTERPRET(filecheck=CHECK):

int pairArity<each T>() where countof(T) == 2
{
    return 22;
}

void main()
{
    // CHECK: r=22
    printf("r=%d\n", pairArity<int, float>());
}
