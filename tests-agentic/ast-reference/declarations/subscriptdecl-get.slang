//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=A SubscriptDecl makes the type indexable; a[i] invokes the subscript's get accessor.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of SubscriptDecl: a `__subscript`
// declaration makes a type indexable so `a[i]` resolves to the
// subscript's accessor body.

//TEST:INTERPRET(filecheck=CHECK):

struct Triple
{
    int a, b, c;
    __subscript(int i) -> int
    {
        get
        {
            if (i == 0) return a;
            if (i == 1) return b;
            return c;
        }
    }
}

void main()
{
    Triple t;
    t.a = 10; t.b = 20; t.c = 30;
    //CHECK: 20
    printf("%d\n", t[1]);
}
