//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Expression checking rejects subscripting a non-array, non-subscriptable type (E30013).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Companion to call-on-non-callable (E30016): expression checking
// also rejects `x[0]` when x has no subscript declarations.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = 5;
    int y = x[0];
//CHECK:     ^ invalid subscript expression
//CHECK:     ^ no subscript declarations found for type 'int'
}
