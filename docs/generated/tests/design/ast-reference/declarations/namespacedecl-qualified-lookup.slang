//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#namespacedecl-moduledecl-filedecl
//META: doc_section_digest=5c73eb281115d6eb7872ab686bf9bf11c3b781484b5a0da42d6602f1bdb3379c
//META: purpose=A NamespaceDecl introduces a named scope; its members are reached with the namespace-qualified name.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a namespace keeps its members out of the enclosing scope and
// reachable through `N::`. Two namespaces declare the same function name with
// different results; the CHECK asserts both, which is only possible if each
// qualified name resolves inside its own namespace.

//TEST:INTERPRET(filecheck=CHECK):

namespace A { int kind() { return 1; } }
namespace B { int kind() { return 2; } }

void main()
{
    //CHECK: 1 2
    printf("%d %d\n", A::kind(), B::kind());
}
