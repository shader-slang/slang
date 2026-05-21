//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<half, 3> instantiation: three half components are independently addressable and elementwise add works.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: half3 covers the N=3 dimension for the half element
// type. Each component is independently observable. As with
// vec2-half-arithmetic, we add the scalar lanes because the
// interpreter lacks a vectorized half-add handler.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    half3 a = half3(half(1.0), half(2.0), half(3.0));
    half3 b = half3(half(4.0), half(5.0), half(6.0));
    half cx = a.x + b.x;
    half cy = a.y + b.y;
    half cz = a.z + b.z;
    // CHECK: c=5,7,9
    printf("c=%d,%d,%d\n", int(cx), int(cy), int(cz));
}
