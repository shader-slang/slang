//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#namespacedecl-moduledecl-filedecl
//META: doc_section_digest=5c73eb281115d6eb7872ab686bf9bf11c3b781484b5a0da42d6602f1bdb3379c
//META: purpose=Multiple textual namespace declarations with the same name in one module are collapsed into one NamespaceDecl during parsing.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that reopening a namespace adds to the existing one rather than
// creating a second, shadowing scope. Two blocks named `N` each declare one
// function; the CHECK asserts a sum computed from both under a single `N::`
// qualifier, which requires the two blocks to have been merged.

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
