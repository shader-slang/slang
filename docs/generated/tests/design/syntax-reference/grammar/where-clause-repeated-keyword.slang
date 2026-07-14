//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=f6e12c33ddd19d9369ee802b5f8dd1f7ec0e74a603076a1958b10edb008910ca
//META: purpose=`WhereClause ::= ('where' WhereTerm)+` — each `where` keyword introduces exactly one term; several constraints repeat the keyword.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the repeated-keyword shape of a multi-constraint where
// clause: `where T : IA where U : IB` states two terms with two
// `where` keywords. The generic body uses a method from each
// constraint, so both terms must have been recorded.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int a(); }
interface IB { int b(); }

struct SA : IA { int a() { return 30; } }
struct SB : IB { int b() { return 12; } }

int combine<T, U>(T t, U u) where T : IA where U : IB
{
    return t.a() + u.b();
}

void main()
{
    SA sa;
    SB sb;
    // CHECK: r=42
    printf("r=%d\n", combine(sa, sb));
}
