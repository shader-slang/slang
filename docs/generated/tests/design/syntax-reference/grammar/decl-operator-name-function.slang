//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#variable--binding-declarations
//META: doc_section_digest=581119c2905797d9e5ca17f62d23d4176de2817cf2cb3cd58cf50bd73f2b8fe3
//META: purpose=`DeclaratorName` accepts the `'operator' OperatorName` form, which is legal on a function declarator.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `'operator' OperatorName` alternative of DeclaratorName by
// declaring `operator+` over a user struct. The declarator is followed by a
// parameter list, which is the branch `allowOperatorName` is granted on, so
// the declaration must survive. Writing `a + b` in the body then resolves to
// that declaration; printing 7 proves the operator name became a callable
// function rather than an ordinary identifier.

//TEST:INTERPRET(filecheck=CHECK):

struct V
{
    int x;
}

V operator+(V a, V b)
{
    V r;
    r.x = a.x + b.x;
    return r;
}

void main()
{
    V a;
    a.x = 3;
    V b;
    b.x = 4;
    V c = a + b;
    // CHECK: r=7
    printf("r=%d\n", c.x);
}
