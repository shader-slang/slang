//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
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
