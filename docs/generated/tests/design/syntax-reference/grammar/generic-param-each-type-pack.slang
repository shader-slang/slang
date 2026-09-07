//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=9ec28a7f9972204cb231849951463cd51e25e934f85ba429e611a1a16838b4e2
//META: purpose=`GenericParam` accepts the type-pack form `'each' IDENT (':' Type)?` (GenericTypePackParamDecl).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `'each' IDENT` arm of GenericParam. The doc distinguishes it
// from `let each` by two-token lookahead: `each T` introduces a *type* pack.
// Three arguments of three different types are passed and the body reports
// `countof(T)`; printing 3 proves one type-pack parameter absorbed them all.

//TEST:INTERPRET(filecheck=CHECK):

int countArgs<each T>(expand each T args)
{
    return countof(T);
}

void main()
{
    // CHECK: r=3
    printf("r=%d\n", countArgs(1, 2.0f, true));
}
