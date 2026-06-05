//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic value parameter with a default value `let N : uint = 4` is used when no explicit value is supplied.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc grammar: `generic-value-param-decl` = `let identifier [: simple-type-spec] [= init-expr]`.
// The `= init-expr` part provides a default value for the value parameter.

//TEST:INTERPRET(filecheck=CHECK):

struct DefaultSized<let N : uint = 4>
{
    int data[N];
}

void main()
{
    // Use the default size of 4
    DefaultSized<> obj;
    obj.data[0] = 1;
    obj.data[1] = 2;
    obj.data[2] = 3;
    obj.data[3] = 4;
    // CHECK: 10
    printf("%d\n", obj.data[0] + obj.data[1] + obj.data[2] + obj.data[3]);
}
