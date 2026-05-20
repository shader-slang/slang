//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=f4322db59ceb8b8af42c196b566ca82e3f4804c47a61babf133a4e241eddf511
//META: purpose=`static` and `const` are simple modifiers; a `static const` file-scope binding is a compile-time-known value the parser accepts and the checker treats as read-only.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `static const int K = 13;` at file scope is the canonical
// compile-time-known-value pattern. Reading it from `main` exercises
// both modifiers on a single declaration.

//TEST:INTERPRET(filecheck=CHECK):

static const int K = 13;

void main()
{
    // CHECK: K=13
    printf("K=%d\n", K);
}
