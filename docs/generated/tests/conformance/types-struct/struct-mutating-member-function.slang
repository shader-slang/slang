//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#nonstatic-member-function
//META: doc_section_digest=c90f11708cb380fb674a4708aaf5743e5a1f31c69b6cf4d7b4a599ffc35791bb
//META: purpose=A member function declared with `[mutating]` may write to non-static data members; without it, write access is disallowed.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "If write access is required, the member function must be declared with the `[mutating]` attribute."
// incrementAndReturnA() uses [mutating] and increments a; the new value is returned and observed.

//TEST:INTERPRET(filecheck=CHECK):

struct TestStruct
{
    int a;

    [mutating] int incrementAndReturnA()
    {
        a = a + 1;
        return a;
    }
}

void main()
{
    TestStruct obj;
    obj.a = 10;
    int result = obj.incrementAndReturnA();
    // CHECK: 11
    printf("%d\n", result);
    // CHECK: 11
    printf("%d\n", obj.a);
}
