//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=When a constructor is present, a non-`const` field's default initializer is overridden by the constructor's explicit assignment.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a constructor's explicit assignment overrides a field's default initializer.
// WithDefault.x has default 99, but __init(int) overwrites it; CHECK confirms the constructed
// value is 42, not 99.

//TEST:INTERPRET(filecheck=CHECK):

struct WithDefault
{
    int x = 99;

    __init(int v)
    {
        x = v;  // overrides the default initializer
    }
}

void main()
{
    WithDefault obj = { 42 };
    // CHECK: 42
    printf("%d\n", obj.x);
}
