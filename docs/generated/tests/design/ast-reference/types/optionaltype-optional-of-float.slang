//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=OptionalType<T> wraps a numeric type T (here float); .hasValue is true for a wrapped value and .value yields it back unchanged.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of OptionalType element-type: Optional<float>
// correctly wraps a float value, exposing it through .value with
// the original type. Complements the existing Optional<int> smoke.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<float> a = 3.5f;
    // CHECK: aHas=1
    printf("aHas=%d\n", a.hasValue ? 1 : 0);
    // CHECK: aV=3
    printf("aV=%d\n", int(a.value));
}
