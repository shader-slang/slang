//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic value parameter declared with the traditional syntax (`uint N` rather than `let N : uint`) is accepted and usable as an array size.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc grammar lists `generic-value-param-trad-decl` as `simple-type-spec identifier [= init-expr]`.
// Both `let N : uint` and `uint N` are valid ways to declare a generic value parameter.

//TEST:INTERPRET(filecheck=CHECK):

struct TradFixed<uint N>
{
    int data[N];
}

void main()
{
    TradFixed<3> obj;
    obj.data[0] = 100;
    obj.data[1] = 200;
    obj.data[2] = 300;
    // CHECK: 600
    printf("%d\n", obj.data[0] + obj.data[1] + obj.data[2]);
}
