//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType<double, 3> instantiation; double3 elementwise add survives evaluation across three components.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: double element type at N=3. Three independent double
// components addressable as .x .y .z.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    double3 a = double3(1.0lf, 2.0lf, 3.0lf);
    double3 b = double3(10.0lf, 20.0lf, 30.0lf);
    double3 c = a + b;
    // CHECK: c=11,22,33
    printf("c=%d,%d,%d\n", int(c.x), int(c.y), int(c.z));
}
