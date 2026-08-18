//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#generics
//META: doc_section_digest=de256a399516188e43bb96d93ea3c68437fd40807db23a0da48c05221ab85f6d
//META: purpose=A generic function with a type parameter can be implicitly specialized to int and float; the function returns the correct typed value for each instantiation.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a generic type parameter `T` can be instantiated with both `int` and `float`.
// `myIdentity` is called once per type; CHECK lines assert 42 and 3.14, confirming
// independent monomorphization for each instantiation.

//TEST:INTERPRET(filecheck=CHECK):

T myIdentity<T>(T val) { return val; }

void main()
{
    int r = myIdentity(42);
    float f = myIdentity(3.14);
    // CHECK: 42
    // CHECK: 3.14
    printf("%d\n%g\n", r, f);
}
