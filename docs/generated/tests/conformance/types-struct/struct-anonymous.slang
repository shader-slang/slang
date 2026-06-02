//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=An anonymous struct (no identifier) is assigned a unique compiler-generated name; the variable declared with it has its fields accessible via dot notation.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an anonymous struct declared inline as a variable type compiles and its fields
// are accessible via dot notation. The interpreter prints obj.a and obj.b; CHECK confirms
// both values match what was assigned.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // Anonymous struct inline variable declaration
    struct { int a; float b; } obj;
    obj.a = 42;
    obj.b = 1.5f;
    // CHECK: 42
    printf("%d\n", obj.a);
    // CHECK: 1.5
    printf("%.1f\n", obj.b);
}
