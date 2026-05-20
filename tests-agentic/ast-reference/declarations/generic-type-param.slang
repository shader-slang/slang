//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
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
