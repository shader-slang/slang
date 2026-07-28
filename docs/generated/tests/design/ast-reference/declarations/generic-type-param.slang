//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A GenericTypeParamDecl introduces a type parameter; the inner decl uses the parameter as a type and is specialized when called.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of GenericTypeParamDecl: the
// parameter `T` introduced by `<T>` is a type usable anywhere a type
// is expected (parameter type, return type, local declaration).

//TEST:INTERPRET(filecheck=CHECK):

T identity<T>(T x) { return x; }

void main()
{
    int   i = identity<int>(11);
    float f = identity<float>(1.5f);
    //CHECK: 11 1.500000
    printf("%d %f\n", i, f);
}
