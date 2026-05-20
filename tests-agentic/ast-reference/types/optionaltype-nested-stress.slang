//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=OptionalType nests: an Optional<Optional<int>> distinguishes outer-none, inner-none, and inner-has-value cases independently.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe: Optional<Optional<int>> exercises two layers of
// OptionalType nesting. The outer hasValue and the inner hasValue
// are independent observation points.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<Optional<int>> a = Optional<int>(7);
    Optional<Optional<int>> b = none;
    // CHECK: outerHas=1
    printf("outerHas=%d\n", a.hasValue ? 1 : 0);
    // CHECK: innerHas=1
    printf("innerHas=%d\n", a.value.hasValue ? 1 : 0);
    // CHECK: innerV=7
    printf("innerV=%d\n", a.value.value);
    // CHECK: bHas=0
    printf("bHas=%d\n", b.hasValue ? 1 : 0);
}
