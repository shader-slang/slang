//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=An OptionalType<T> distinguishes a wrapped value from no-value; .hasValue and .value expose the state.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `OptionalType` is the BuiltinType for `Optional<T>`. The
// user-facing claim: an `Optional<int>` initialized to `42` has
// `.hasValue == true` and `.value == 42`, while one initialized to
// `none` has `.hasValue == false`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> a = 42;
    Optional<int> b = none;
    // CHECK: a.hasValue=1
    printf("a.hasValue=%d\n", a.hasValue ? 1 : 0);
    // CHECK: a.value=42
    printf("a.value=%d\n", a.value);
    // CHECK: b.hasValue=0
    printf("b.hasValue=%d\n", b.hasValue ? 1 : 0);
}
