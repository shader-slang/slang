//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
//META: purpose=`interface` is the decl keyword that declares a protocol-style requirement set; a `struct` conforms via `:` and supplies the required member.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declare an interface, conform a struct to it, and dispatch via a
// generic function constrained by that interface. The result proves
// `interface` is bound to its documented role and that the parser
// accepts the conformance clause.

//TEST:INTERPRET(filecheck=CHECK):

interface IAnswer
{
    int get();
}

struct Forty : IAnswer
{
    int get() { return 40; }
}

int call<T : IAnswer>(T t) { return t.get() + 2; }

void main()
{
    Forty f;
    // CHECK: r=42
    printf("r=%d\n", call(f));
}
