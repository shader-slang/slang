//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
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
