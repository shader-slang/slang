//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
//META: purpose=`let` introduces an immutable binding and `var` introduces a mutable binding; both are decl keywords.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declare both a `let` and a `var` of the same value, mutate the
// `var` once, and observe the result. The mutability difference is
// covered by a separate diagnostic test.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    let a = 7;
    var b = 5;
    b = b + 1;
    // CHECK: a=7 b=6
    printf("a=%d b=%d\n", a, b);
}
