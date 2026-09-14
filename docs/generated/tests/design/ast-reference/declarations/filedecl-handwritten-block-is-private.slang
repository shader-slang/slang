//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#namespacedecl-moduledecl-filedecl
//META: doc_section_digest=0c426eaac3adce684d6e1bcd65a3863432d05ffa3258277a4308ff2a6c6f3c36
//META: purpose=A hand-written __file_decl block gets the same private scope any other container gets, so its members are not visible to the rest of the file; the transparency belongs to the file scopes the compiler builds instead.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the distinction the doc draws between the node and the scopes the
// include machinery builds. `__file_decl` names the same class a `__include`
// produces, so the obvious reading is that anything inside it joins the file's
// own scope. It does not: the block is an ordinary container with a private
// scope, and only the FileDecl recorded by an include gets spliced into the
// module's lookup chain. CHECK pins E30015 at the call, which is exactly the
// failure a reader who assumed transparency would not expect.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

__file_decl
{
    int helper() { return 7; }
}

void main()
{
    printf("%d\n", helper());
//CHECK:           ^^^^^^ E30015
}
