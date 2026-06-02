//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=A nested type declared inside a struct body is enclosed within the struct's name scope and accessed via the outer struct name with the dot operator.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a type nested inside a struct is accessed by chaining the outer struct name
// with the dot operator. obj2 is declared as TestStruct.NestedStruct; CHECK confirms d=55.

//TEST:INTERPRET(filecheck=CHECK):

struct TestStruct
{
    struct NestedStruct
    {
        int d;
    }
}

void main()
{
    TestStruct.NestedStruct obj2;
    obj2.d = 55;
    // CHECK: 55
    printf("%d\n", obj2.d);
}
