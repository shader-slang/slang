//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=ArrayExpressionType with a half3 element type stores half-vector elements addressable per array index.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: ArrayExpressionType with element-type =
// VectorExpressionType<half, 3>. Confirms a fixed-size array
// can carry half-vector elements. Each element is independently
// indexable.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    half3 arr[2] = {
        half3(half(1.0), half(2.0), half(3.0)),
        half3(half(4.0), half(5.0), half(6.0))
    };
    // CHECK: a0z=3 a1x=4
    printf("a0z=%d a1x=%d\n", int(arr[0].z), int(arr[1].x));
}
