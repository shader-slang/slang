//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=367c8c8287c8144f80e46f8a9f2f52ead869866080d544fadcb2763d0d63b953
//META: purpose=`WhereClause` parses as `'where' WhereTerm (',' WhereTerm)*` with `WhereTerm ::= Type ':' Type` for conformance.
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
