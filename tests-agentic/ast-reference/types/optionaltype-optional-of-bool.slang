//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=OptionalType<T> wraps a bool; the wrapped truthiness is recoverable via .value, distinct from the no-value sentinel .hasValue=false.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the bool element-type: Optional<bool> can hold
// `true` (the value is recovered) or `none` (hasValue false). The
// type's `hasValue == false` is distinct from the value `false`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<bool> a = true;
    Optional<bool> b = none;
    // CHECK: aV=1
    printf("aV=%d\n", a.value ? 1 : 0);
    // CHECK: aHas=1
    printf("aHas=%d\n", a.hasValue ? 1 : 0);
    // CHECK: bHas=0
    printf("bHas=%d\n", b.hasValue ? 1 : 0);
}
