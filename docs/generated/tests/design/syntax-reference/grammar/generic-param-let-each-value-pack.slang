//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=9ec28a7f9972204cb231849951463cd51e25e934f85ba429e611a1a16838b4e2
//META: purpose=`GenericParam` accepts the value-pack form `'let' 'each' IDENT (':' Type)?` (GenericValuePackParamDecl).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `'let' 'each' IDENT ':' Type` arm, the value-pack sibling of
// the plain `let` value parameter. Three explicit value arguments are
// supplied and the body reports `countof(D)`; printing 3 proves the pack
// parameter absorbed all three values as one variadic binding.

//TEST:INTERPRET(filecheck=CHECK):

int packArity<let each D : int>()
{
    return countof(D);
}

void main()
{
    // CHECK: r=3
    printf("r=%d\n", packArity<10, 20, 30>());
}
