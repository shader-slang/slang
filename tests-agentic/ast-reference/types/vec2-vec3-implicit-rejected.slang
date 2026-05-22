//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=A VectorExpressionType<float, 2> cannot be implicitly assigned to a VectorExpressionType<float, 3> binding; the dim mismatch is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative expansion: two VectorExpressionType instantiations
// that share an element type but differ in count are distinct
// types. The checker rejects the implicit narrow-to-wide
// assignment.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    float2 v2 = float2(1.0, 2.0);
    float3 v3 = v2;
//CHECK:        ^^ type mismatch in expression
// CHECK: E30019
}
