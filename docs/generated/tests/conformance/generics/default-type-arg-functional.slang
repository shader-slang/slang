//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic type parameter with a default type `T = int` can be used without an explicit type argument; the default is used when omitted.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc grammar shows `generic-type-param-decl` as `[typename] identifier [: simple-type-spec] [= simple-type-spec]`.
// The `= simple-type-spec` part is an optional default type.

//TEST:INTERPRET(filecheck=CHECK):

struct Box<T = int>
{
    T value;
}

void main()
{
    // Use the default (int)
    Box<> defaultBox;
    defaultBox.value = 55;

    // Explicit override
    Box<float> floatBox;
    floatBox.value = 2.0;

    // CHECK: 55
    printf("%d\n", defaultBox.value);
    // CHECK: 2
    printf("%d\n", (int)floatBox.value);
}
