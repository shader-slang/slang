//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=f6e12c33ddd19d9369ee802b5f8dd1f7ec0e74a603076a1958b10edb008910ca
//META: purpose=`WhereClause` parses as `('where' WhereTerm)+` with the conformance form `Type ':' Type` as a single term.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the WhereClause production using the conformance form
// `where T : IPing` as a postfix to a generic function declaration.

//TEST:INTERPRET(filecheck=CHECK):

interface IPing { int ping(); }
struct A : IPing { int ping() { return 17; } }

int callPing<T>(T t) where T : IPing { return t.ping(); }

void main()
{
    A a;
    // CHECK: r=17
    printf("r=%d\n", callPing<A>(a));
}
