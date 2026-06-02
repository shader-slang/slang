//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#nonstatic-member-function
//META: doc_section_digest=c90f11708cb380fb674a4708aaf5743e5a1f31c69b6cf4d7b4a599ffc35791bb
//META: purpose=Inside a non-static member function, `this.member` and bare `member` both refer to the same object field; the `this.` prefix is optional.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `this.member` and bare `member` are equivalent inside a non-static method.
// sumExplicit() uses `this.x + this.y`; sumImplicit() uses `x + y`; both are called on the
// same instance and CHECK confirms both return the same sum.

//TEST:INTERPRET(filecheck=CHECK):

struct Vec2
{
    int x;
    int y;

    int sumExplicit()
    {
        return this.x + this.y;
    }

    int sumImplicit()
    {
        return x + y;
    }
}

void main()
{
    Vec2 v;
    v.x = 3;
    v.y = 4;
    int r1 = v.sumExplicit();
    int r2 = v.sumImplicit();
    // CHECK: 7
    printf("%d\n", r1);
    // CHECK: 7
    printf("%d\n", r2);
}
