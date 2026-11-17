//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:39+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#sibling-scopes
//META: doc_section_digest=48e2e716cb98e52f251b42e53f41f2630e81baad625d5ff212284e8c3ac35e05
//META: purpose=A qualified namespace declaration pushes a scope for the earlier component, so the body of namespace A.B reaches members of A unqualified.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// For a qualified name such as `namespace A.B`, the parser also pushes a
// scope for each earlier component that the body must see. The body of
// `A.B` therefore resolves `base()` (a member of A) with no qualification.

//TEST:INTERPRET(filecheck=CHECK):

namespace A {
    int base() { return 5; }
}

namespace A.B {
    int derived() { return base() * 2; }
}

void main()
{
    // CHECK: d=10
    printf("d=%d\n", A::B::derived());
}
