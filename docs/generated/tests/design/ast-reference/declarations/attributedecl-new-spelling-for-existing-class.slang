//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#syntaxdecl-and-the-syntax-as-declaration-model
//META: doc_section_digest=b82bc3789ca4f01d0fc3ccd18b12c8825ec02726f8c57106748d528cf4de33e7
//META: purpose=attribute_syntax binds a new spelling to an AST attribute class the compiler already knows, and the attribute's behaviour comes from that class rather than from anything the declaration says.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the declaration supplies only a name. `[MyNoDiscard]` is a
// spelling this file invents, and nothing about it says what discarding a
// result should do; the whole behaviour is inherited from the
// `NoDiscardAttribute` class named after the colon. The proof is that the
// invented spelling produces the *same* diagnostic the built-in `[NoDiscard]`
// does — E30059 at the discarded call — which could not happen if the parser
// were doing anything attribute-specific with the new name. The
// `__attributeTarget` line is what tells the checker where the spelling may
// be placed; without it every use is rejected with E31002 instead.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

__attributeTarget(FunctionDeclBase)
attribute_syntax [MyNoDiscard] : NoDiscardAttribute;

[MyNoDiscard]
int produce()
{
    return 1;
}

void main()
{
    produce();
//CHECK:   ^ E30059
}
