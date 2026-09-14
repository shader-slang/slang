//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=9ec28a7f9972204cb231849951463cd51e25e934f85ba429e611a1a16838b4e2
//META: purpose=`GenericParam` accepts the type-parameter form `'typename'? IDENT (':' Type)?` with a single supertype after the colon.
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
