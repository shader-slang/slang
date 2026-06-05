//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=f4322db59ceb8b8af42c196b566ca82e3f4804c47a61babf133a4e241eddf511
//META: purpose=`public` and `private` are simple visibility modifiers; a `private` member is still reachable from another member of the same type.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `public struct` holds a `public` method and a `private` method.
// The third method (default visibility) calls both. The result
// confirms the parser accepted both visibility modifiers and that the
// in-type call from `sum` to `priv` is allowed.

//TEST:INTERPRET(filecheck=CHECK):

public struct Box
{
    public int pub() { return 7; }
    private int priv() { return 9; }
    int sum() { return pub() + priv(); }
}

void main()
{
    Box b;
    // CHECK: s=16
    printf("s=%d\n", b.sum());
}
