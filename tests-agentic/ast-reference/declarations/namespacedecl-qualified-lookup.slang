//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#namespacedecl-moduledecl-filedecl
//META: doc_section_digest=081eefb5df95b1052109f01da319f6c20dfdae3022986c491d04ad23688d1558
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
