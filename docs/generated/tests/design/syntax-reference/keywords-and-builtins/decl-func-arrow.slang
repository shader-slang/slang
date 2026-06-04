//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
//META: purpose=`func name(params) -> RetType { ... }` is the Slang-style function-decl form introduced by the `func` decl keyword.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Use the `func ... -> T` form (as opposed to the C-style
// `T name(...)` form) and observe the resulting function dispatches
// normally. Both `func` and the right-arrow trivia token are part of
// this decl form.

//TEST:INTERPRET(filecheck=CHECK):

func answer() -> int { return 42; }

void main()
{
    // CHECK: a=42
    printf("a=%d\n", answer());
}
