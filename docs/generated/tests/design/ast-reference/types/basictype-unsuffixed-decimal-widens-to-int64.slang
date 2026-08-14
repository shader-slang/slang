//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=eb9e797ca9c3dbb949a62072e232852c0f820f49dd331388c5e7ffcde24a8c1c
//META: purpose=An unsuffixed decimal integer literal takes the narrowest basic type that holds it - int first, then int64 - so a value one past int MAX picks int64 rather than wrapping or picking uint.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the front end's literal-typing rule at the exact int/int64 seam.
// The type of a literal is not directly observable, so three overloads act as
// a probe and report which one the call binds to. 2147483647 is int MAX and
// must still bind the `int` overload; 2147483648 is one past it and must bind
// the `int64_t` overload rather than the `uint` one, which is what "narrowest
// type that holds the value - int, then int64" means for an unsuffixed
// decimal. A uint result would show up as 3 and prove the widening order
// wrong.

//TEST:INTERPRET(filecheck=CHECK):

int probe(int x)
{
    return 1;
}
int probe(int64_t x)
{
    return 2;
}
int probe(uint x)
{
    return 3;
}

void main()
{
    printf("intmax=%d intmaxplus1=%d\n", probe(2147483647), probe(2147483648));
}

//CHECK: intmax=1 intmaxplus1=2
