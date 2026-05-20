//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
//META: purpose=`typealias Name = T;` is a decl keyword binding `Name` to type `T`; the alias resolves to its underlying type for overload resolution.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `typealias MyInt = int;` then call a `kind(int)` overload through
// the alias. The overload that matches confirms `MyInt` resolved to
// `int`.

//TEST:INTERPRET(filecheck=CHECK):

typealias MyInt = int;

int kind(int v) { return 1; }
int kind(uint v) { return 2; }

void main()
{
    MyInt x = 42;
    // CHECK: kind=1
    printf("kind=%d\n", kind(x));
}
