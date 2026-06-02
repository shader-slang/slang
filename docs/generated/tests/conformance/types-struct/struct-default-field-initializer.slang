//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=When an initializer list does not provide a value for a non-static field that has a default initializer, the default is used.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "When an object is initialized using an initializer list, the default initializer
// of a non-static data member specifies the initial value when the initializer list does not provide one."

//TEST:INTERPRET(filecheck=CHECK):

struct Defaults
{
    int x = 99;
    int y;
    int z = 77;
}

void main()
{
    // Initializer list provides x and y but not z; z should fall back to 77
    Defaults d = { 1, 2 };
    // CHECK: 1
    printf("%d\n", d.x);
    // CHECK: 2
    printf("%d\n", d.y);
    // CHECK: 77
    printf("%d\n", d.z);
}
