//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
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
