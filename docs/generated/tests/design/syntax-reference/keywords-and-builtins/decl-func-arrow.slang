//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
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
