//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A GenericTypeParamDecl introduces a type parameter usable anywhere a type is expected inside the generic.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the name introduced by `<T>` is a usable type in the parameter
// list, the return type and locals. The generic is instantiated at two
// different types and the CHECK asserts both round-tripped values.

//TEST:INTERPRET(filecheck=CHECK):

T identity<T>(T x) { return x; }

void main()
{
    int   i = identity<int>(11);
    float f = identity<float>(1.5f);
    //CHECK: 11 1.500000
    printf("%d %f\n", i, f);
}
