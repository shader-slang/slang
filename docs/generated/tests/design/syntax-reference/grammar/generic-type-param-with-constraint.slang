//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=f6e12c33ddd19d9369ee802b5f8dd1f7ec0e74a603076a1958b10edb008910ca
//META: purpose=`GenericParam` accepts the type-parameter form `IDENT ':' TypeList`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `GenericParam ::= IDENT (':' TypeList)?` production by
// declaring a generic function with a constrained type parameter
// (`T : IPing`) and calling it with a conforming type.

//TEST:INTERPRET(filecheck=CHECK):

interface IPing { int ping(); }

struct A : IPing { int ping() { return 99; } }

int callPing<T : IPing>(T t) { return t.ping(); }

void main()
{
    A a;
    // CHECK: r=99
    printf("r=%d\n", callPing<A>(a));
}
