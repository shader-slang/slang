//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#nonstatic-member-function
//META: doc_section_digest=c90f11708cb380fb674a4708aaf5743e5a1f31c69b6cf4d7b4a599ffc35791bb
//META: purpose=A non-static member function has a hidden `this` parameter; it may read (but not write) non-static members by default.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "By default, only a read access to the object members is allowed by a member function."
// getA() reads this.a and returns it; the test verifies the read happens correctly.

//TEST:INTERPRET(filecheck=CHECK):

struct TestStruct
{
    int a;

    int getA()
    {
        return this.a;
    }
}

void main()
{
    TestStruct obj;
    obj.a = 42;
    // CHECK: 42
    printf("%d\n", obj.getA());
}
