//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=OptionalType<double2> wraps a vector element type; .hasValue is true for a wrapped vector value and .value yields it back unchanged.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: Optional<T> with T = VectorExpressionType<double, 2>.
// Verifies that the BuiltinType OptionalType can wrap a vector
// of a non-default-float element type.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<double2> a = double2(3.5lf, 4.5lf);
    // CHECK: aHas=1
    printf("aHas=%d\n", a.hasValue ? 1 : 0);
    double2 v = a.value;
    // CHECK: v=3,4
    printf("v=%d,%d\n", int(v.x), int(v.y));
}
