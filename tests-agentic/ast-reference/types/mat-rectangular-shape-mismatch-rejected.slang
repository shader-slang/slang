//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=A MatrixExpressionType<float, 2, 3> cannot be implicitly assigned to a MatrixExpressionType<float, 3, 2>; the (rows, cols) operands distinguish the two types.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative expansion: two MatrixExpressionType instantiations
// that share an element type and total cell count but differ in
// (rows, cols) are distinct types. The checker rejects the
// cross-shape assignment.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    float2x3 a = float2x3(1, 2, 3, 4, 5, 6);
    float3x2 b = a;
//CHECK:         ^ type mismatch in expression
// CHECK: E30019
}
