//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=56c06b08f61a5eac3a9a9fa44d74ee850f8a6841b9cad895376978a437f1148f
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
