//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:59:02+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#namespacedecl-moduledecl-filedecl
//META: doc_section_digest=5c73eb281115d6eb7872ab686bf9bf11c3b781484b5a0da42d6602f1bdb3379c
//META: purpose=A qualified namespace name (`namespace A.B`, `namespace A::B`) is expanded into nested namespaces, so members are reachable through the qualified path.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both qualified-name spellings declare nested namespaces. Functions
// are declared under `namespace A.B` and `namespace C::D` and called through
// `A::B::` and `C::D::`; CHECK asserts the sum, reachable only if the
// qualified names were expanded into nested namespaces.

//TEST:INTERPRET(filecheck=CHECK):

namespace A.B
{
    int f() { return 3; }
}

namespace C::D
{
    int g() { return 4; }
}

void main()
{
    //CHECK: v=7
    printf("v=%d\n", A::B::f() + C::D::g());
}
