//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=ArrayExpressionType with a double2 element type carries double-vector elements; the array is indexable per element.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: ArrayExpressionType with element-type =
// VectorExpressionType<double, 2>. Confirms 64-bit-float vector
// elements survive the array-of-vector composition.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    double2 arr[3] = {
        double2(1.0lf, 2.0lf),
        double2(3.0lf, 4.0lf),
        double2(5.0lf, 6.0lf)
    };
    // CHECK: a=1,3,5
    printf("a=%d,%d,%d\n", int(arr[0].x), int(arr[1].x), int(arr[2].x));
}
