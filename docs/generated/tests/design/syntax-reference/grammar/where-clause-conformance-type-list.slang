//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=f6e12c33ddd19d9369ee802b5f8dd1f7ec0e74a603076a1958b10edb008910ca
//META: purpose=A conformance `WhereTerm` may list several supertypes after one colon: `Type ':' Type (',' Type)*`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the comma-continued supertype list inside a single
// conformance term: `where T : IA, IB` constrains one parameter to
// both interfaces with one `where` keyword. The body calls a method
// from each interface on the same value.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int a(); }
interface IB { int b(); }

struct S : IA, IB
{
    int a() { return 40; }
    int b() { return 2; }
}

int both<T>(T t) where T : IA, IB
{
    return t.a() + t.b();
}

void main()
{
    S s;
    // CHECK: r=42
    printf("r=%d\n", both(s));
}
