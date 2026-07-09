//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#namespacedecl-moduledecl-filedecl
//META: doc_section_digest=d4f66ae4ecd0c9db07c2bf1b96051aba8c0c48fb0aa33c09268b87a2b7e1d6f4
//META: purpose=Multiple textual namespace declarations with the same name in one module are collapsed into one NamespaceDecl during parsing.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented collapsing of same-named namespace blocks.
// `namespace N` is opened twice; members declared across both blocks
// are visible under a single `N::` qualifier — confirming the two
// blocks are collapsed into one NamespaceDecl.

//TEST:INTERPRET(filecheck=CHECK):

namespace N
{
    int first() { return 7; }
}

namespace N
{
    int second() { return 35; }
}

void main()
{
    //CHECK: sum=42
    printf("sum=%d\n", N::first() + N::second());
}
