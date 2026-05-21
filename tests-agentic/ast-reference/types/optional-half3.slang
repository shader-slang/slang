//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=OptionalType<half3> wraps a half-vector; the wrapped value is observable through .value and .hasValue is true.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: Optional<T> with T = VectorExpressionType<half, 3>.
// Confirms a narrow-float vector can be wrapped by OptionalType.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<half3> a = half3(half(1.0), half(2.0), half(3.0));
    // CHECK: aHas=1
    printf("aHas=%d\n", a.hasValue ? 1 : 0);
    half3 v = a.value;
    // CHECK: v=1,2,3
    printf("v=%d,%d,%d\n", int(v.x), int(v.y), int(v.z));
}
