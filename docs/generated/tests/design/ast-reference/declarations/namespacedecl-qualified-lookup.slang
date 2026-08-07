//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#namespacedecl-moduledecl-filedecl
//META: doc_section_digest=d4f66ae4ecd0c9db07c2bf1b96051aba8c0c48fb0aa33c09268b87a2b7e1d6f4
//META: purpose=A NamespaceDecl introduces a named scope; members are reached with the namespace-qualified name.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of NamespaceDecl: a member declared
// inside `namespace N { ... }` is reachable as `N::f`. Two functions
// with the same name in different namespaces stay distinct.

//TEST:INTERPRET(filecheck=CHECK):

namespace A { int kind() { return 1; } }
namespace B { int kind() { return 2; } }

void main()
{
    //CHECK: 1 2
    printf("%d %d\n", A::kind(), B::kind());
}
