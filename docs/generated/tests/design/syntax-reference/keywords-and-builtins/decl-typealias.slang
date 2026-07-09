//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=e3d06ef8bf7a4508b0ecb0515b3867979078821ac9b8143e7b039cd76e1b73b1
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
