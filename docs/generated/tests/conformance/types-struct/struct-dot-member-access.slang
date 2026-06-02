//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#accessing-members-and-nested-types
//META: doc_section_digest=2755043638ff4d80438587ffde5a02e52e4e6ff81f851f26621452c5c5afad95
//META: purpose=Static and non-static struct members and nested types are accessed using the dot `.` operator; `::` is deprecated for static member access.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "The static and non-static structure members and nested types are accessed using `.`"
// Tests access of: non-static field via obj.a, static field via TestStruct.b, nested type via TestStruct.NestedStruct.

//TEST:INTERPRET(filecheck=CHECK):

struct TestStruct
{
    int a;
    static const int b = 5;

    struct NestedStruct
    {
        static const int c = 6;
    }

    int getA()
    {
        return this.a;
    }
}

void main()
{
    TestStruct obj = { };
    obj.a = 42;

    // Non-static field via dot
    // CHECK: 42
    printf("%d\n", obj.a);

    // Static const via type name dot
    // CHECK: 5
    printf("%d\n", TestStruct.b);

    // Nested static via chained dots
    // CHECK: 6
    printf("%d\n", TestStruct.NestedStruct.c);

    // Member function via dot
    // CHECK: 42
    printf("%d\n", obj.getA());
}
