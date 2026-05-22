//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=A decimal literal that exceeds the largest 64-bit integer type produces a documented "integer literal is too large" diagnostic at the BasicExpressionType binding site.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative boundary probe: assign a literal too large for any
// BasicExpressionType integer (here, uint64_t) and check the
// checker rejects it. Twenty 9s exceed the largest 64-bit unsigned
// (about 1.8e19) by an order of magnitude.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    uint64_t x = 99999999999999999999;
//CHECK: integer literal is too large
//CHECK: E10012
}
