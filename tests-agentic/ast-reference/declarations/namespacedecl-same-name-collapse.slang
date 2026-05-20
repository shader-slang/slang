//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#namespacedecl-moduledecl-filedecl
//META: doc_section_digest=081eefb5df95b1052109f01da319f6c20dfdae3022986c491d04ad23688d1558
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
